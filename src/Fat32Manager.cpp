#include <Fat32Manager.hpp>

Fat32Manager::Fat32Manager( ) :
	fatInfoLoaded( false ),
	bootSectorLoaded( false ),
	fatTableLoaded( false ),
	initOk( false )
{}

Fat32Manager::Fat32Manager( const std::wstring &partitionPath ) :
	fatInfoLoaded( false ),
	bootSectorLoaded( false ),
	fatTableLoaded( false )
{
	mappedFileMngr.setFilePath( partitionPath );
	initOk = init( );
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

Fat32Manager::FreeSpaceChunk::FreeSpaceChunk( uintmax_t offset, size_t size ) :
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
	LOG( INFO ) << "Initializating";

	if( loadBootSector() == false )
	{
		LOG( INFO ) << "Loading boot sector failed. Returning false";
		return false;
	}

	loadFatInfo();

	if( loadFatTable() == false )
	{
		LOG( INFO ) << "Loading fat table failed. Returning false";
		return false;
	}

	LOG( INFO ) << "Initializating successed. Returning true";
	
	return true;
}

bool Fat32Manager::init( )
{
	LOG( INFO ) << "Initializating";

	return _init();
}

void Fat32Manager::clear( )
{
	LOG( INFO ) << "Clearing ";
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

	LOG( INFO ) << "Loading boot sector";

	if( bootSectorLoaded )
	{
		LOG( INFO ) << "Boot sector was already loaded. Returning true";
		return true;
	}

	LOG( INFO ) << "Mapping boot sector";
	mappedPtr = reinterpret_cast<FatBS*>( mappedFileMngr.map( 0, bootSectorSize ) );

	if( mappedPtr == nullptr )
	{
		LOG( INFO ) << "Mapped ptr == nullptr. Returning false";
		return false;
	}

	bootSector = *mappedPtr;

	loadFat32ExtBS();

	bootSectorLoaded = true;

	LOG( INFO ) << "Loadding successed. Returning true";

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

	uintmax_t clusterStart = getClusterStartOffset( clusterNo );

	mappedPtr = mappedFileMngr.map( getClusterStartOffset( clusterNo ),
									clusterSize( ) );
	return mappedPtr;
}

inline size_t Fat32Manager::clusterSize( ) const
{
	return bootSector.bytes_per_sector * bootSector.sectors_per_cluster;
}
inline size_t Fat32Manager::getClusterFirstSectorNo( size_t clusterNo ) const
{
	return ( clusterNo - 2 ) * bootSector.sectors_per_cluster + fatInfo.first_data_sector;
}
inline uintmax_t Fat32Manager::getClusterStartOffset( size_t clusterNo ) const
{
	return getClusterFirstSectorNo( clusterNo ) * bootSector.bytes_per_sector;
}

std::vector<FatRawLongFileName> Fat32Manager::extractLongFileNames( char *&ptrInCluster ) const
{
	FatRawLongFileName *longFileNamePtr;
	char *charPtr;
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

std::vector<size_t> Fat32Manager::getClusterChain( size_t firstCluster )
{
	std::vector<size_t> chain;
	size_t cluster = firstCluster;

	do
	{
		chain.push_back( cluster );
		cluster = fatTable[cluster];
	} while( cluster <= lastClusterMagic );

	return chain;
}
std::vector<DirectoryEntry> Fat32Manager::getDirEntriesFromDirCluster( size_t dirCluster )
{
	std::vector<DirectoryEntry> dirEntries;
	FatRawDirectoryEntry tempRawDirEntry;
	std::vector<FatRawLongFileName> tempRawLongFileNames;
	char *mappedClusterPtr, *endOfCluster;

	mappedClusterPtr = loadCluster( dirCluster );

	if( mappedClusterPtr == nullptr )
		return dirEntries;

	endOfCluster = mappedClusterPtr + clusterSize( );

	while( true )
	{
		tempRawLongFileNames = extractLongFileNames( mappedClusterPtr );

		if( mappedClusterPtr >= endOfCluster || *( static_cast<char*>( mappedClusterPtr ) ) == 0 )
			break;

		tempRawDirEntry = *( reinterpret_cast<FatRawDirectoryEntry*>( mappedClusterPtr ) );

		dirEntries.push_back( DirectoryEntry( tempRawLongFileNames, tempRawDirEntry ) );

		mappedClusterPtr += sizeof( FatRawDirectoryEntry );
	}

	return dirEntries;
}
std::vector<DirectoryEntry> Fat32Manager::getDirEntriesFromFolder( size_t firstCluster )
{
	std::vector<size_t> clusterChain;
	std::vector<DirectoryEntry> tmpDirEntries, dirEntries;

	clusterChain = getClusterChain( firstCluster );

	for( const auto &cluster : clusterChain )
	{
		tmpDirEntries = getDirEntriesFromDirCluster( cluster );
		dirEntries.reserve( dirEntries.size( ) + tmpDirEntries.size( ) );
		dirEntries.insert( dirEntries.end( ), tmpDirEntries.begin( ), tmpDirEntries.end( ) );
	}

	return dirEntries;
}
std::vector<std::wstring> Fat32Manager::getPathFoldersNames( const std::wstring &path ) const
{
	std::vector<std::wstring> folders;
	size_t posBegin = 0, posEnd, numberToCopy;

	while( ( posEnd = path.find( '/', posBegin ) ) != std::wstring::npos )
	{
		numberToCopy = posEnd - posBegin;
		folders.push_back( path.substr( posBegin, numberToCopy ) );
		posBegin = posEnd + 1;
	}

	return folders;
}
std::wstring Fat32Manager::getPathFileName( const std::wstring &path ) const
{
	size_t pos = path.find_last_of( '/' );

	pos = ( pos == std::string::npos ) ? 0 : pos + 1; // pos+1 to not copy '/' char

	return path.substr( pos );
}
size_t Fat32Manager::getFreeSpaceAfterFile( const DirectoryEntry &fileDirEntry ) const
{
	return clusterSize( ) - ( fileDirEntry.getFileSize( ) % clusterSize( ) );
}
size_t Fat32Manager::getFileLastClusterNo( const DirectoryEntry &fileDirEntry ) const
{
	size_t currentClusterNo;

	currentClusterNo = fileDirEntry.getCluster( );

	while( true )
	{
		if( fatTable[currentClusterNo] >= lastClusterMagic )
			return currentClusterNo;

		currentClusterNo = fatTable[currentClusterNo];
	}
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

	LOG( INFO ) << "Finding next dir entry";
	LOG( INFO ) << "Prev dir entry: " << prevDirEntry;

	dirEntries = getDirEntriesFromFolder( folderCluster );

	auto it = dirEntries.begin( );

	if( prevDirEntry.type( ) != BAD_DIR_ENTRY )
		it = std::find( dirEntries.begin( ), dirEntries.end( ), prevDirEntry ) + 1;

	if( it == dirEntries.end() )
	{
		LOG( INFO ) << "Dir entry not found. Returning bad dir entry";
		return DirectoryEntry();
	}
	else
	{
		LOG( INFO ) << "Returning found dir entry: " << *it;
		return *it;
	}

	LOG( INFO ) << "Dir entry not found. Returning bad dir entry";
	return DirectoryEntry( );
}
DirectoryEntry Fat32Manager::findDirEntryInFolder( std::wstring searchedDirEntryName, const size_t folderCluster )
{
	DirectoryEntry currentDirEntry;
	std::wstring dirEntryName;

	LOG( INFO ) << "\nFinding dir entry in folder. \nSearched dir entry name: " << searchedDirEntryName << "\nFolder first cluster: " << folderCluster;

	boost::to_upper( searchedDirEntryName );

	do
	{
		dirEntryName = currentDirEntry.getName( );
		boost::to_upper( dirEntryName );

		if( dirEntryName == searchedDirEntryName )
		{
			LOG( INFO ) << "Returning found dir entry: " << currentDirEntry;
			return currentDirEntry;
		}

		currentDirEntry = findNextDirEntry( folderCluster, currentDirEntry );

	} while( currentDirEntry.type( ) != BAD_DIR_ENTRY );

	LOG( INFO ) << "Dir entry not found. Returning bad dir entry";

	return DirectoryEntry();
}
DirectoryEntry Fat32Manager::findFile( const std::wstring &path )
{
	std::vector<std::wstring> foldersNames;
	std::wstring fileName;
	DirectoryEntry currentFolder, foundFile;

	LOG( INFO ) << "Finding file: " << path;

	foldersNames = getPathFoldersNames( path );
	fileName = getPathFileName( path );

	currentFolder.setCluster( fat32ExtBS.root_cluster );

	LOG( INFO ) << "Following folders";

	for( const auto &folderName : foldersNames )
	{
		currentFolder = findDirEntryInFolder( folderName, currentFolder.getCluster( ) );

		LOG( INFO ) << folderName;

		if( currentFolder.type() == BAD_DIR_ENTRY )
		{
			LOG( INFO ) << folderName << "not found. Returning bad dir entry";
			return DirectoryEntry();
		}
	}

	LOG( INFO ) << "Searching for last entry: " << fileName;

	if( fileName.length() == 0 )
	{
		LOG( INFO ) << "Last entry name empty. Returning bad dir entry";
		return DirectoryEntry();
	}

	foundFile = findDirEntryInFolder( fileName, currentFolder.getCluster( ) );

	if( foundFile.type() == BAD_DIR_ENTRY )
	{
		LOG( INFO ) << fileName << " not found. Returning bad dir entry";
		return DirectoryEntry();
	}

	LOG( INFO ) << "Returning found file: " << foundFile;

	return foundFile;
}

void Fat32Manager::setPartitionPath( const fs::path &partitionPath )
{
	LOG( INFO ) << "Setting partition path: " << partitionPath;
	mappedFileMngr.setFilePath( partitionPath );

	initOk = init();
}

bool Fat32Manager::isValidFat32( )
{
	if( !initOk )
	{
		LOG( INFO ) << "Fat manager not initialized correctly. Returning false";
		return false;
	}

	LOG( INFO ) << "Returning " << std::boolalpha << ( getFatType() == FAT32 );

	return getFatType( ) == FAT32;
}

void Fat32Manager::close( )
{
	LOG( INFO ) << "Closing";
	mappedFileMngr.close( );
}

bool Fat32Manager::isPathCorrect( const std::wstring &path )
{
	bool isCorrect = findFile( path ).type() != BAD_DIR_ENTRY;

	LOG( INFO ) << "Is path: " << path << " correct = " << std::boolalpha << isCorrect;

	return isCorrect;
}

EFatType Fat32Manager::getFatType( )
{
	if( fatInfo.total_clusters < 4085 )
	{
		LOG( INFO ) << "Returning fat12";
		return FAT12;
	}
	else
	{
		if( fatInfo.total_clusters < 65525 )
		{
			LOG( INFO ) << "Returning fat16";
			return FAT16;
		}
		else
		{
			LOG( INFO ) << "Returning fat32";
			return FAT32;
		}
	}
}

size_t Fat32Manager::getFreeSpaceAfterFile( const std::wstring &path )
{
	DirectoryEntry file;

	LOG( INFO ) << "Getting size of free space after file: " << path;

	file = findFile( path );

	LOG( INFO ) << "found file: " << file;
	LOG( INFO ) << "Returning " << getFreeSpaceAfterFile( file );

	return getFreeSpaceAfterFile( file );
}

size_t Fat32Manager::getFileLastClusterNo( const std::wstring &path )
{
	DirectoryEntry file;

	LOG( INFO ) << "Getting las cluster no of file: " << path;

	file = findFile( path );

	LOG( INFO ) << "found file: " << file;

	LOG( INFO ) << "Returning " << getFileLastClusterNo( file );

	return getFileLastClusterNo( file );
}

size_t Fat32Manager::getFileFreeSpaceOffset( const std::wstring &path )
{
	DirectoryEntry file;

	LOG( INFO ) << "Getting free space offset of file: " << path;

	file = findFile( path );

	LOG( INFO ) << "found file: " << file;

	LOG( INFO ) << "Returning " << file.getFileSize() % clusterSize();

	return file.getFileSize() % clusterSize();
}

std::vector<Fat32Manager::FreeSpaceChunk> Fat32Manager::getSpacesAfterFiles( const std::vector<std::wstring> &files )
{
	std::vector<FreeSpaceChunk> chunks;
	el::Logger* defaultLogger = el::Loggers::getLogger( "default" );

	LOG( INFO ) << "Getting spaces after files";

	for( const auto &file : files )
	{
		LOG( INFO ) << "Getting free space info from file: " << file.c_str();

		size_t clusterNo = getFileLastClusterNo( file );

		chunks.push_back( FreeSpaceChunk(	getClusterStartOffset( clusterNo ),
											getFreeSpaceAfterFile( file ) ) );
	}

	defaultLogger->info( "Collected chunks: %v", chunks );

	return chunks;
}

char* Fat32Manager::mapSpaceAfterFiles( const std::vector<std::wstring> &files )
{
	std::vector<ClusterWithFreeSpace> clusters;
	uintmax_t preparedOffset, preparedSize;
	size_t firstCluster, lastCluster;

	LOG( INFO ) << "Mapping space after files";

	for( const auto &file : files )
	{
		clusters.push_back( ClusterWithFreeSpace( getFileLastClusterNo( file ),
			getFileFreeSpaceOffset( file ) ) );
	}

	firstCluster = std::min_element( clusters.begin( ), clusters.end( ) )->clusterNo;
	lastCluster = std::max_element( clusters.begin( ), clusters.end( ) )->clusterNo;

	preparedOffset = getClusterStartOffset( firstCluster );
	preparedSize = getClusterStartOffset( lastCluster + 1 ) - preparedOffset;

	LOG( INFO ) << "Mapping variables: \n\
				   first cluster = " << firstCluster << "\n\
				   last cluster = " << lastCluster << "\n\
				   prepared offset = " << preparedOffset << "\n\
				   preparedSize = " << preparedSize;

	return mappedFileMngr.map( preparedOffset, preparedSize, true );
}

std::ostream& operator<<( std::ostream &out, const std::vector<Fat32Manager::FreeSpaceChunk> &v )
{
	for( const auto &i : v )
		out << i;

	return out;
}

std::ostream& operator<< ( std::ostream &out, const FatBS &bs )
{
	out << "\tbootjmp = 0x" << std::hex;
	for( size_t i = 0; i < 3; ++i )
		out << std::hex << static_cast<size_t>( bs.bootjmp[i] );

	out << "\n\toem_name = ";
	for( size_t i = 0; i < 8; ++i )
		out << bs.oem_name[i];

	out << std::dec;

	out << "\n\tbytes_per_sector = " << bs.bytes_per_sector\
		<< "\n\tsectors_per_cluster = " << static_cast<size_t>( bs.sectors_per_cluster )\
		<< "\n\treserved_sector_count = " << bs.reserved_sector_count\
		<< "\n\ttable_count = " << static_cast<size_t>( bs.table_count )\
		<< "\n\troot_entry_count = " << bs.root_entry_count\
		<< "\n\ttotal_sectors_16 = " << bs.total_sectors_16\
		<< "\n\tmedia_type = 0x" << std::hex << static_cast<size_t>( bs.media_type )\
		<< "\n\ttable_size_16 = " << std::dec << bs.table_size_16\
		<< "\n\tsectors_per_track = " << bs.sectors_per_track\
		<< "\n\thead_side_count = " << bs.head_side_count\
		<< "\n\thidden_sector_count = " << bs.hidden_sector_count\
		<< "\n\ttotal_sectors_32 = " << bs.total_sectors_32;

	return out;
}

std::ostream& operator<< ( std::ostream &out, const Fat32ExtBS &bs )
{
	out << "\n\ttable_size_32 = " << bs.table_size_32\
		<< "\n\textended_flags = " << bs.extended_flags\
		<< "\n\tfat_version = " << bs.fat_version\
		<< "\n\troot_cluster = " << bs.root_cluster\
		<< "\n\tfat_info = " << bs.fat_info\
		<< "\n\tbackup_BS_sector = " << bs.backup_BS_sector;

	out << "\n\treserved_0 =" << std::hex;
	for( size_t i = 0; i < 12; ++i )
		out << " 0x" << static_cast<size_t>( bs.reserved_0[i] );

	out << std::dec;
	out << "\n\tdrive_number = " << static_cast<size_t>( bs.drive_number );

	out << "\n\treserved_1 = 0x" << std::hex << static_cast<size_t>( bs.reserved_1 )\
		<< "\n\tboot_signature = 0x" << static_cast<size_t>( bs.boot_signature )\
		<< "\n\tvolume_id = 0x" << bs.volume_id;

	out << "\n\tvolume_label = ";
	for( size_t i = 0; i < 12; ++i )
		out << bs.volume_label[i];

	out << "\n\tfat_type_label = ";
	for( size_t i = 0; i < 8; ++i )
		out << bs.fat_type_label[i];

	return out;
}


std::ostream& operator<< ( std::ostream &out, const FatInfo &bs )
{
	out << std::dec;
	out << "\n\ttotal_sectors = " << bs.total_clusters\
		<< "\n\tfat_size = " << bs.fat_size\
		<< "\n\troot_dir_sectors = " << bs.root_dir_sectors\
		<< "\n\tfirst_data_sector = " << bs.first_data_sector\
		<< "\n\tfirst_fat_sector = " << bs.first_fat_sector\
		<< "\n\tdata_sectors = " << bs.data_sectors;

	return out;
}

std::ostream& operator<< ( std::ostream &out, const Fat32Manager &fm )
{
	out << "\nBoot sector:" << fm.bootSector \
		<< "\nExtended boot sector: " << fm.fat32ExtBS \
		<< "\nFat info: " << fm.fatInfo;

	return out;;
}

