#ifndef _INCLUDE_FAT32MANAGER_
#define _INCLUDE_FAT32MANAGER_

#include <memory>
#include <vector>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost\filesystem\operations.hpp>


#include <FatStructs.h>
#include <MappedFileManager.hpp>
#include <DirectoryEntry.hpp>

namespace fs = boost::filesystem;

class Fat32Manager
{
private:
	static const size_t bootSectorSize = 512;
	static const unsigned char DELETED_MAGIC = 0xE5;
	static const unsigned char LFN_ATTRIBUTE = 0x0F;
	const size_t lastClusterMagic = 0x0FFFFFF8;

	struct ClusterWithFreeSpace
	{
		size_t clusterNo,
		freeSpaceOffset;

		ClusterWithFreeSpace( ) {}
		ClusterWithFreeSpace( size_t clusterNo, size_t freeSpaceOffset ) :
			clusterNo( clusterNo ),
			freeSpaceOffset( freeSpaceOffset )
		{}

		bool operator< ( const ClusterWithFreeSpace &c )
		{
			return clusterNo < c.clusterNo;
		}
		bool operator> ( const ClusterWithFreeSpace &c )
		{
			return clusterNo > c.clusterNo;
		}
	};

	

	FatBS bootSector;
	Fat32ExtBS fat32ExtBS;
	FatInfo fatInfo;
	bool fatInfoLoaded, bootSectorLoaded, fatTableLoaded, initOk;
	MappedFileManager mappedFileMngr;
	std::vector<uint32_t> fatTable;
	

	//private methods
private:

	void loadFat32ExtBS()
	{
		fat32ExtBS = *( reinterpret_cast<Fat32ExtBS*>( &bootSector.extended_section ) );
	}

	bool loadBootSector()
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

	void loadFatInfo()
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

	bool loadFatTable()
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

	char* loadCluster( size_t clusterNo )
	{
		char *mappedPtr;

		uintmax_t clusterStart = getClusterStartOffset( clusterNo );

		mappedPtr = mappedFileMngr.map( getClusterStartOffset( clusterNo ),
										clusterSize() );
		return mappedPtr;
	}

	inline size_t clusterSize() const
	{ 
		return bootSector.bytes_per_sector * bootSector.sectors_per_cluster;
	}
	inline size_t getClusterFirstSectorNo( size_t clusterNo ) const
	{
		return ( clusterNo - 2 ) * bootSector.sectors_per_cluster + fatInfo.first_data_sector;
	}
	inline uintmax_t getClusterStartOffset( size_t clusterNo ) const
	{
		return getClusterFirstSectorNo( clusterNo ) * bootSector.bytes_per_sector;
	}

	std::vector<FatRawLongFileName> extractLongFileNames( char *&ptrInCluster ) const
	{
		FatRawLongFileName *longFileNamePtr;
		char *charPtr;
		std::vector<FatRawLongFileName> ret;

		longFileNamePtr = reinterpret_cast<FatRawLongFileName*>( ptrInCluster );
		
		if( *ptrInCluster == 0 || *ptrInCluster == DELETED_MAGIC )
			return ret;

		for( ; longFileNamePtr->attribute == LFN_ATTRIBUTE; ++longFileNamePtr )
			ret.push_back( *longFileNamePtr );

		std::sort( ret.begin(), 
				   ret.end(),
				   []( const FatRawLongFileName &a, const FatRawLongFileName &b )
		{
			return a.sequenceNumber < b.sequenceNumber;
		} );

		ptrInCluster = reinterpret_cast<char*>( longFileNamePtr );

		return ret;
	}
	
	std::vector<size_t> getClusterChain( size_t firstCluster )
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

	std::vector<DirectoryEntry> getDirEntriesFromDirCluster( size_t dirCluster )
	{
		std::vector<DirectoryEntry> dirEntries;
		FatRawDirectoryEntry tempRawDirEntry;
		std::vector<FatRawLongFileName> tempRawLongFileNames;
		char *mappedClusterPtr, *endOfCluster;

		mappedClusterPtr = loadCluster( dirCluster );

		if( mappedClusterPtr == nullptr )
			return dirEntries;

		endOfCluster = mappedClusterPtr + clusterSize();

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

	std::vector<DirectoryEntry> getDirEntriesFromFolder( size_t firstCluster )
	{
		std::vector<size_t> clusterChain;
		std::vector<DirectoryEntry> tmpDirEntries, dirEntries;

		clusterChain = getClusterChain( firstCluster );

		for( const auto &cluster : clusterChain )
		{
			tmpDirEntries = getDirEntriesFromDirCluster( cluster );
			dirEntries.reserve( dirEntries.size() + tmpDirEntries.size() );
			dirEntries.insert( dirEntries.end(), tmpDirEntries.begin(), tmpDirEntries.end() );
		}

		return dirEntries;
	}

	std::vector<std::wstring> getPathFoldersNames( const std::wstring &path ) const
	{
		std::vector<std::wstring> folders;
		size_t posBegin = 0, posEnd, numberToCopy;

		while( ( posEnd = path.find( '/', posBegin ) ) != std::wstring::npos )
		{
			numberToCopy = posEnd - posBegin;
			folders.push_back( path.substr( posBegin, numberToCopy ) );
			posBegin = posEnd+1;
		}

		return folders;
	}

	std::wstring getPathFileName( const std::wstring &path ) const
	{
		size_t pos = path.find_last_of( '/' );

		pos = ( pos == std::wstring::npos ) ? 0 : pos + 1; // pos+1 to not copy '/' char

		return path.substr( pos );
	}

	std::wstring removeExtension( const std::wstring& filename )
	{
		size_t lastdot = filename.find_last_of( '.' );

		if( lastdot == std::string::npos ) 
			return filename;

		return filename.substr( 0, lastdot );
	}

	DirectoryEntry findNextDirEntry( size_t folderCluster, const DirectoryEntry &prevDirEntry = DirectoryEntry() )
	{
		std::vector<DirectoryEntry> dirEntries;

		dirEntries = getDirEntriesFromFolder( folderCluster );

		auto it = dirEntries.begin( );

		if( prevDirEntry.type() != BAD_DIR_ENTRY )
			it = std::find( dirEntries.begin( ), dirEntries.end( ), prevDirEntry ) + 1;

		if( it == dirEntries.end() )
			return DirectoryEntry();
		else
			return *it;

		return DirectoryEntry();
	}

	DirectoryEntry findNextFile( size_t folderCluster, const DirectoryEntry &prevDirEntry = DirectoryEntry() )
	{
		return DirectoryEntry();
	}

	DirectoryEntry findDirEntryInFolder( std::wstring searchedDirEntryName, const size_t folderCluster )
	{
		DirectoryEntry currentDirEntry;
		std::wstring dirEntryName;

		boost::to_upper( searchedDirEntryName );

		do
		{
			dirEntryName = currentDirEntry.getName();
			boost::to_upper( dirEntryName );

			if( dirEntryName == searchedDirEntryName )
				return currentDirEntry;

			currentDirEntry = findNextDirEntry( folderCluster, currentDirEntry );

		} while( currentDirEntry.type( ) != BAD_DIR_ENTRY );

		return currentDirEntry;
	}

	bool init()
	{
		if( loadBootSector() == false )
			return false;

		loadFatInfo( );

		if( loadFatTable() == false )
			return false;

		return true;
	}

	void clear()
	{
		fatInfoLoaded = false;
		bootSectorLoaded = false;
		fatTableLoaded = false;
		initOk = false;
	}

	size_t getFreeSpaceAfterFile( const DirectoryEntry &fileDirEntry ) const
	{
		return clusterSize() - ( fileDirEntry.getFileSize() % clusterSize() );
	}

	size_t getFileLastClusterNo( const DirectoryEntry &fileDirEntry ) const
	{

		size_t actualClusterNo;

		actualClusterNo = fileDirEntry.getCluster();

		while( true )
		{
			if( fatTable[actualClusterNo] >= lastClusterMagic )
				return actualClusterNo;

			actualClusterNo = fatTable[actualClusterNo];
		}
	}

	ClusterInfo getFileLastClusterInfo( const DirectoryEntry &fileDirEntry )
	{
		ClusterInfo ret;

		ret.freeBytes = getFreeSpaceAfterFile( fileDirEntry );
		ret.freeBytesOffset = fileDirEntry.getFileSize() % clusterSize();
		ret.clusterNo = getFileLastClusterNo( fileDirEntry );

		return ret;
	}

	DirectoryEntry findFile( const std::wstring &path )
	{
		std::vector<std::wstring> foldersNames;
		std::wstring fileName;
		DirectoryEntry currentFolder, findedFile;

		foldersNames = getPathFoldersNames( path );
		fileName = getPathFileName( path );

		currentFolder.setCluster( fat32ExtBS.root_cluster );

		for( const auto &folderName : foldersNames )
		{
			currentFolder = findDirEntryInFolder( folderName, currentFolder.getCluster( ) );

			if( currentFolder.type( ) == BAD_DIR_ENTRY )
				return DirectoryEntry();
		}

		if( fileName.length() == 0 )
			return DirectoryEntry();
		
		findedFile = findDirEntryInFolder( fileName, currentFolder.getCluster() );

		if( findedFile.type( ) == BAD_DIR_ENTRY )
			return DirectoryEntry( );

		return findedFile;
	}
	
public:
	struct FreeSpaceChunk
	{
		uintmax_t offset;
		size_t size;

		FreeSpaceChunk( ) {}
		FreeSpaceChunk( uintmax_t offset, size_t size ) :
			offset( offset ),
			size( size )
		{}

		bool operator< ( const FreeSpaceChunk &c )
		{
			return offset < c.offset;
		}
	};

	Fat32Manager() :
		fatInfoLoaded( false ),
		bootSectorLoaded( false ),
		fatTableLoaded( false ),
		initOk( false )
	{}

	Fat32Manager( const std::string &partitionPath ) :
		fatInfoLoaded( false ),
		bootSectorLoaded( false ),
		fatTableLoaded( false )
	{
		mappedFileMngr.setFilePath( partitionPath );
		initOk = init();
	}

	~Fat32Manager() {}

	void setPartitionPath( const fs::path &partitionPath )
	{
		mappedFileMngr.setFilePath( partitionPath );
		initOk = init();
	}

	EFatType getFatType( )
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

	bool isValidFat32( )
	{
		if( !initOk )
			return false;

		return getFatType( ) == FAT32;
	}

	void printFiles()
	{
		std::vector<DirectoryEntry> dirEntries;

		if( !initOk )
			return;

		dirEntries = getDirEntriesFromDirCluster( fat32ExtBS.root_cluster );

		for( auto &i : dirEntries )
			i.print( std::wcout );
	}

	std::vector<ClusterInfo> getClustersWithFreeBytes( const uintmax_t freeBytesNeeded, 
													   const size_t metadataSize )
	{
		const size_t acceptatbleFreeBytes = 10 + metadataSize; // instead of 10 you can write any sensible number of bytes

		std::vector<ClusterInfo> clusters;
		intmax_t leftBytesToFind;
		DirectoryEntry tempDirEntry;

		leftBytesToFind = freeBytesNeeded;

		while( leftBytesToFind > 0 )
		{
			size_t freeBytes;

			tempDirEntry = findNextFile( fat32ExtBS.root_cluster, tempDirEntry );

			if( tempDirEntry.type() == BAD_DIR_ENTRY )
				return std::vector<ClusterInfo>();

			freeBytes = getFreeSpaceAfterFile( tempDirEntry );
			
			if( freeBytes >= acceptatbleFreeBytes )
			{
				clusters.push_back( getFileLastClusterInfo( tempDirEntry ) );
				leftBytesToFind -= freeBytes - metadataSize;
			}
		}

		return clusters;
	}

	void writeToCluster( const size_t clusterNo, 
						 const size_t offset, 
						 const size_t dataSize, 
						 const char *data )
	{
		char *clusterPtr = loadCluster( clusterNo );

		std::copy( data, data + dataSize, clusterPtr + offset );
	}




	void close()
	{
		mappedFileMngr.close();
	}

	bool isPathCorrect( const std::wstring &path )
	{
		return findFile( path ).type() != BAD_DIR_ENTRY;
	}

	size_t getFreeSpaceAfterFile( const std::wstring &path )
	{
		DirectoryEntry file;

		file = findFile( path );

		return getFreeSpaceAfterFile( file );
	}

	size_t getFileLastClusterNo( const std::wstring &path )
	{
		DirectoryEntry file;

		file = findFile( path );

		return getFileLastClusterNo( file );
	}

	size_t getFileFreeSpaceOffset( const std::wstring &path )
	{
		DirectoryEntry file;

		file = findFile( path );

		return file.getFileSize() % clusterSize();
	}

	char* mapSpaceAfterFiles( const std::vector<std::wstring> &files )
	{
		std::vector<ClusterWithFreeSpace> clusters;
		uintmax_t preparedOffset, preparedSize;
		size_t firstCluster, lastCluster;

		for( const auto &file : files )
		{
			clusters.push_back( ClusterWithFreeSpace( getFileLastClusterNo( file ),
													getFileFreeSpaceOffset( file ) ) );
		}

		firstCluster = std::min_element( clusters.begin(), clusters.end() )->clusterNo;
		lastCluster = std::max_element( clusters.begin( ), clusters.end( ) )->clusterNo;

		preparedOffset = getClusterStartOffset( firstCluster );
		preparedSize = getClusterStartOffset( lastCluster + 1 ) - preparedOffset;

		return mappedFileMngr.map( preparedOffset, preparedSize, true );
	}

	std::vector<FreeSpaceChunk> getSpacesAfterFiles( const std::vector<std::wstring> &files )
	{
		std::vector<FreeSpaceChunk> chunks;

		for( const auto &file : files )
		{
			size_t clusterNo = getFileLastClusterNo(file);

			chunks.push_back( FreeSpaceChunk( getClusterStartOffset( clusterNo ),
												getFreeSpaceAfterFile( file ) ) );
		}

		return chunks;
	}
};

#endif