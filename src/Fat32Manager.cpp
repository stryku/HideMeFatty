#include <Fat32Manager.hpp>
#include <QDir>

Fat32Manager::Fat32Manager( ) :
	fatInfoLoaded( false ),
	bootSectorLoaded( false ),
	fatTableLoaded( false ),
	initOk( false )
{}

Fat32Manager::Fat32Manager( const QString &partitionPath ) :
	fatInfoLoaded( false ),
	bootSectorLoaded( false ),
	fatTableLoaded( false )
{
    mappedFileMngr.setFilePath( partitionPath.toStdString() );
	initOk = _init();
}

Fat32Manager::ClusterWithFreeSpace::ClusterWithFreeSpace( size_t clusterNo,
														  size_t freeSpaceOffset ) :
	clusterNo( clusterNo ),
	freeSpaceOffset( freeSpaceOffset )
{}

bool Fat32Manager::ClusterWithFreeSpace::operator< ( const ClusterWithFreeSpace &c )
{
	return clusterNo < c.clusterNo;
}
bool Fat32Manager::ClusterWithFreeSpace::operator>( const ClusterWithFreeSpace &c )
{
	return clusterNo > c.clusterNo;
}

Fat32Manager::FreeSpaceChunk::FreeSpaceChunk( uint64_t offset, size_t size ) :
	offset( offset ),
	size( size )
{}

std::ostream& operator<< ( std::ostream &out, const Fat32Manager::FreeSpaceChunk &fsc )
{
	out << "offset = " << fsc.offset << ", size = " << fsc.size;
	return out;
}

bool Fat32Manager::FreeSpaceChunk::operator< ( const FreeSpaceChunk &c ) const
{
	return offset < c.offset;
}

bool Fat32Manager::_init( )
{
	if( loadBootSector() == false )
		return false;

	loadFatInfo();

	if( loadFatTable() == false )
		return false;

	return true;
}

void Fat32Manager::init( )
{
	initOk = _init();
}

bool Fat32Manager::good( )
{
	return initOk;
}

void Fat32Manager::clear()
{
	mappedFileMngr.close();
	fatInfoLoaded = false;
	bootSectorLoaded = false;
	fatTableLoaded = false;
	initOk = false;
}

void Fat32Manager::loadFat32ExtBS( )
{
	fat32ExtBS = *( reinterpret_cast<Fat32ExtBS*>( &bootSector.extended_section ) );
}

bool Fat32Manager::loadBootSector( )
{
	FatBS *mappedPtr;

	if( bootSectorLoaded )
		return true;

	mappedPtr = reinterpret_cast<FatBS*>( mappedFileMngr.map( 0, bootSectorSize ) );

	if( mappedPtr == nullptr )
		return false;

	bootSector = *mappedPtr;

	loadFat32ExtBS();

	bootSectorLoaded = true;

	return true;
}

void Fat32Manager::loadFatInfo()
{
	if( fatInfoLoaded )
		return;

	fatInfo.total_sectors = ( bootSector.total_sectors_16 == 0 ) ? bootSector.total_sectors_32 : bootSector.total_sectors_16;
	fatInfo.fat_size = ( bootSector.table_size_16 == 0 ) ? fat32ExtBS.table_size_32 : bootSector.table_size_16;
	fatInfo.root_dir_sectors = ( ( bootSector.root_entry_count * 32 ) + ( bootSector.bytes_per_sector - 1 ) ) / bootSector.bytes_per_sector;
	fatInfo.first_data_sector = bootSector.reserved_sector_count + ( bootSector.table_count * fatInfo.fat_size ) + fatInfo.root_dir_sectors;
	fatInfo.first_fat_sector = bootSector.reserved_sector_count;
	fatInfo.data_sectors = fatInfo.total_sectors - ( bootSector.reserved_sector_count + ( bootSector.table_count * fatInfo.fat_size ) + fatInfo.root_dir_sectors );
	fatInfo.total_clusters = fatInfo.data_sectors / bootSector.sectors_per_cluster;

	fatInfoLoaded = true;
}

bool Fat32Manager::loadFatTable()
{
	char *mappedPtr;
	size_t fatTableSize;

	if( bootSectorLoaded == false )
		return false;

	if( fatTableLoaded )
		return true;

	fatTableSize = fatInfo.fat_size * bootSector.bytes_per_sector;

	mappedPtr = mappedFileMngr.map( fatInfo.first_fat_sector * bootSector.bytes_per_sector,
									fatTableSize );

	if( mappedPtr == nullptr )
		return false;

	fatTable.resize( fatTableSize / sizeof( uint32_t ) );

	std::copy( mappedPtr,
			   mappedPtr + fatInfo.fat_size * bootSector.bytes_per_sector,
			   reinterpret_cast<char*>( &fatTable[0] ) );

	fatTableLoaded = true;

	return true;
}
char* Fat32Manager::loadCluster( size_t clusterNo )
{
	char *mappedPtr;

	mappedPtr = mappedFileMngr.map( getClusterStartOffset( clusterNo ),
									clusterSize( ) );
	return mappedPtr;
}

size_t Fat32Manager::clusterSize( ) const
{
	return bootSector.bytes_per_sector * bootSector.sectors_per_cluster;
}

inline size_t Fat32Manager::getClusterFirstSectorNo( size_t clusterNo ) const
{
	return ( clusterNo - 2 ) * bootSector.sectors_per_cluster + fatInfo.first_data_sector;
}

inline uint64_t Fat32Manager::getClusterStartOffset( size_t clusterNo ) const
{
	return getClusterFirstSectorNo( clusterNo ) * bootSector.bytes_per_sector;
}

std::vector<FatRawLongFileName> Fat32Manager::extractLongFileNames( char *&ptrInCluster ) const
{
	FatRawLongFileName *longFileNamePtr;
	std::vector<FatRawLongFileName> ret;

	longFileNamePtr = reinterpret_cast<FatRawLongFileName*>( ptrInCluster );

	if( *ptrInCluster == 0 || *ptrInCluster == DELETED_MAGIC )
		return ret;

	for( ; longFileNamePtr->attribute == LFN_ATTRIBUTE; ++longFileNamePtr )
		ret.push_back( *longFileNamePtr );

	std::sort( ret.begin( ),
			   ret.end( ),
			   []( const FatRawLongFileName &a, const FatRawLongFileName &b )
                {
                    return a.sequenceNumber < b.sequenceNumber;
                } );

	ptrInCluster = reinterpret_cast<char*>( longFileNamePtr );

	return ret;
}

std::vector<size_t> Fat32Manager::getClusterChain( size_t firstCluster ) const
{
	std::vector<size_t> chain;
	size_t cluster = firstCluster;

	do
	{
		chain.push_back( cluster );
		cluster = fatTable[cluster];
    } while( cluster < lastClusterMagic );

	return chain;
}

std::vector<DirectoryEntry> Fat32Manager::getDirEntriesFromFolder( size_t firstCluster )
{
    std::vector<size_t> clusterChain;
    std::vector<char> rawFolder;

    clusterChain = getClusterChain( firstCluster );
    rawFolder = getRawFolder( clusterChain );

    return getDirEntriesFromRawFolder( rawFolder );
}

QStringList Fat32Manager::getPathFoldersNames( QString path ) const
{
    auto pos = path.lastIndexOf( "/" );

    if( pos == -1 )
        return QStringList();

    path.truncate( path.lastIndexOf( "/" ) );

    return path.split( "/" );
}

size_t Fat32Manager::getFreeSpaceAfterFile( const DirectoryEntry &fileDirEntry ) const
{
	return clusterSize( ) - ( fileDirEntry.getFileSize( ) % clusterSize( ) );
}

size_t Fat32Manager::getFileLastClusterNo( const DirectoryEntry &fileDirEntry ) const
{
    auto clustersChain = getClusterChain( fileDirEntry.getCluster() );

    return clustersChain.back();
}

ClusterInfo Fat32Manager::getFileLastClusterInfo( const DirectoryEntry &fileDirEntry )
{
	ClusterInfo ret;

	ret.freeBytes = getFreeSpaceAfterFile( fileDirEntry );
	ret.freeBytesOffset = fileDirEntry.getFileSize( ) % clusterSize( );
	ret.clusterNo = getFileLastClusterNo( fileDirEntry );

	return ret;
}

DirectoryEntry Fat32Manager::findNextDirEntry( size_t folderCluster, const DirectoryEntry &prevDirEntry )
{
	std::vector<DirectoryEntry> dirEntries;

	dirEntries = getDirEntriesFromFolder( folderCluster );

	auto it = dirEntries.begin( );

	if( prevDirEntry.type( ) != BAD_DIR_ENTRY )
		it = std::find( dirEntries.begin( ), dirEntries.end( ), prevDirEntry ) + 1;

	if( it == dirEntries.end() )
		return DirectoryEntry();
	else
		return *it;

	return DirectoryEntry( );
}

DirectoryEntry Fat32Manager::findDirEntryInFolder( QString searchedDirEntryName, const size_t folderCluster )
{
	DirectoryEntry currentDirEntry;
    QString dirEntryName;

    searchedDirEntryName = searchedDirEntryName.toUpper();

	do
	{
        dirEntryName = currentDirEntry.getName( );
        dirEntryName = dirEntryName.toUpper();

		if( dirEntryName == searchedDirEntryName )
			return currentDirEntry;

		currentDirEntry = findNextDirEntry( folderCluster, currentDirEntry );

	} while( currentDirEntry.type( ) != BAD_DIR_ENTRY );

	return DirectoryEntry();
}

DirectoryEntry Fat32Manager::getFileParentFolder( const QString &path )
{
    DirectoryEntry currentFolder;
    QStringList foldersNames;

    foldersNames = getPathFoldersNames( path );

    currentFolder.setCluster( fat32ExtBS.root_cluster );

    for( const auto &folderName : foldersNames )
    {
        if(folderName.length() == 0)
            break;

        currentFolder = findDirEntryInFolder( folderName, currentFolder.getCluster( ) );

        if( currentFolder.type() == BAD_DIR_ENTRY )
            return DirectoryEntry();
    }

    return currentFolder;
}

DirectoryEntry Fat32Manager::findFile( const QString &path )
{
    QString fileName;
    DirectoryEntry parentFolder, foundFile;

    fileName = path.split( "/" ).back();

	if( fileName.length() == 0 )
		return DirectoryEntry();

    parentFolder = getFileParentFolder( path );

    if( parentFolder.type() == BAD_DIR_ENTRY )
        return DirectoryEntry();

    foundFile = findDirEntryInFolder( fileName, parentFolder.getCluster( ) );

	if( foundFile.type() == BAD_DIR_ENTRY )
		return DirectoryEntry();

	return foundFile;
}

void Fat32Manager::setPartitionPath( const QString &partitionPath )
{
    mappedFileMngr.setFilePath( partitionPath.toStdString() );
}

bool Fat32Manager::isValidFat32()
{
	if( !initOk )
		return false;

	return getFatType() == FAT32;
}

void Fat32Manager::close()
{
	mappedFileMngr.close();
}

EFatType Fat32Manager::getFatType()
{
	if( fatInfo.total_clusters < 4085 )
		return FAT12;
	else
	{
		if( fatInfo.total_clusters < 65525 )
			return FAT16;
		else
			return FAT32;
	}
}

size_t Fat32Manager::getFreeSpaceAfterFile( const QString &path )
{
    return AdvancedFileInfo( path, clusterSize() ).freeSpaceAfterFile;
}

size_t Fat32Manager::getFileLastClusterNo( const QString &path )
{
	DirectoryEntry file;

	file = findFile( path );

	return getFileLastClusterNo( file );
}

size_t Fat32Manager::getFileFreeSpaceOffset( const DirectoryEntry &file )
{
    return file.getFileSize() % clusterSize();
}

char *Fat32Manager::mapChunks( std::vector<Fat32Manager::FreeSpaceChunk> chunks)
{
    uint64_t preparedOffset, preparedSize;
    Fat32Manager::FreeSpaceChunk firstCluster, lastCluster;

    firstCluster = *( std::min_element( chunks.begin(), chunks.end() ) );
    lastCluster = *( std::max_element( chunks.begin(), chunks.end() ) );

    preparedOffset = firstCluster.offset;
    preparedSize = lastCluster.offset + lastCluster.size - preparedOffset;

    return mappedFileMngr.map( preparedOffset, preparedSize, true );
}

std::vector<Fat32Manager::FreeSpaceChunk> Fat32Manager::getSpacesAfterFiles( const QStringList &files )
{
	std::vector<FreeSpaceChunk> chunks;

	for( const auto &file : files )
    {
        auto fileDirEntry = findFile( file );
        size_t clusterNo = getFileLastClusterNo( fileDirEntry );
        auto startOffset = getClusterStartOffset( clusterNo ),
             freeSpace = getFreeSpaceAfterFile( fileDirEntry ),
             freeSpaceOffset = getFileFreeSpaceOffset( fileDirEntry );

        chunks.push_back( FreeSpaceChunk( startOffset + freeSpaceOffset,
                                          freeSpace ) );
	}

	return chunks;
}

std::vector<char> Fat32Manager::getRawFolder( const std::vector<size_t> &folderClusterChain )
{
    std::vector<char> ret;
    size_t size;
    char *rawPtr, *mappedClusterPtr;

    size = folderClusterChain.size() * clusterSize();

    ret.resize( size );

    rawPtr = ret.data();

    for( const auto &clusterNo : folderClusterChain )
    {
        mappedClusterPtr = loadCluster( clusterNo );

        std::memcpy( rawPtr, mappedClusterPtr, clusterSize() );
        rawPtr += clusterSize();
    }

    return ret;
}

std::vector<DirectoryEntry> Fat32Manager::getDirEntriesFromRawFolder( std::vector<char> &rawFolder )
{
    std::vector<DirectoryEntry> dirEntries;
    FatRawDirectoryEntry tempRawDirEntry;
    std::vector<FatRawLongFileName> tempRawLongFileNames;
    char *rawPtr, *endOfFolder;

    endOfFolder = rawFolder.data() + rawFolder.size();
    rawPtr = rawFolder.data();

    while( true )
    {
        tempRawLongFileNames = extractLongFileNames( rawPtr );

        if( rawPtr >= endOfFolder || *rawPtr == 0 )
            break;

        tempRawDirEntry = *( reinterpret_cast<FatRawDirectoryEntry*>( rawPtr ) );

        dirEntries.push_back( DirectoryEntry( tempRawLongFileNames, tempRawDirEntry ) );

        rawPtr += sizeof( FatRawDirectoryEntry );
    }

    return dirEntries;
}

std::ostream& operator<<( std::ostream &out, const std::vector<Fat32Manager::FreeSpaceChunk> &v )
{
	for( const auto &i : v )
		out << i;

	return out;
}

std::ostream& operator<< ( std::ostream &out, const Fat32Manager &fm )
{
	out << "\nBoot sector:" << fm.bootSector \
		<< "\nExtended boot sector: " << fm.fat32ExtBS \
		<< "\nFat info: " << fm.fatInfo;

	return out;
}

