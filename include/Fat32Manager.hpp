#ifndef _INCLUDE_FAT32MANAGER_
#define _INCLUDE_FAT32MANAGER_

#include <memory>
#include <vector>
#include <algorithm>

#include <FatStructs.h>
#include <MappedFileManager.hpp>
#include <DirectoryEntry.hpp>

class Fat32Manager
{
private:
	static const size_t bootSectorSize = 512;
	static const unsigned char DELETED_MAGIC = 0xE5;
	static const unsigned char LFN_ATTRIBUTE = 0x0F;

	FatBS bootSector;
	Fat32ExtBS fat32ExtBS;
	FatInfo fatInfo;
	bool fatInfoLoaded, bootSectorLoaded, fatTableLoaded, initOk;
	MappedFileManager mappedFileMngr;
	std::vector<unsigned int> fatTable;
	

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

		mappedPtr = mappedFileMngr.map( fatInfo.first_fat_sector,
										fatTableSize );

		if( mappedPtr == nullptr )
			return false;

		fatTable.resize( fatTableSize );

		std::copy( mappedPtr,
				   mappedPtr + fatInfo.fat_size * bootSector.bytes_per_sector,
				   fatTable.begin() );

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
	inline uint64_t getClusterStartOffset( size_t clusterNo ) const
	{
		return getClusterFirstSectorNo( clusterNo ) * bootSector.bytes_per_sector;
	}

	std::vector<FatRawLongFileName> extractLongFileNames( void *&ptrInCluster ) const
	{
		FatRawLongFileName *longFileNamePtr;
		char *charPtr;
		std::vector<FatRawLongFileName> ret;

		longFileNamePtr = static_cast<FatRawLongFileName*>( ptrInCluster );
		charPtr = static_cast<char*>( ptrInCluster );
		
		if( *charPtr == 0 || *charPtr == DELETED_MAGIC )
			return ret;

		for( ; longFileNamePtr->attribute == LFN_ATTRIBUTE; ++longFileNamePtr )
			ret.push_back( *longFileNamePtr );

		std::sort( ret.begin(), 
				   ret.end(),
				   []( const FatRawLongFileName &a, const FatRawLongFileName &b )
		{
			return a.sequenceNumber < b.sequenceNumber;
		} );

		ptrInCluster = longFileNamePtr;

		return ret;
	}
	
	std::vector<DirectoryEntry> getDirEntriesFromDirCluster( size_t dirCluster )
	{
		std::vector<DirectoryEntry> dirEntries;
		FatRawDirectoryEntry tempRawDirEntry;
		std::vector<FatRawLongFileName> tempRawLongFileNames;
		void *mappedClusterPtr;

		mappedClusterPtr = loadCluster( dirCluster );

		if( mappedClusterPtr == nullptr )
			return dirEntries;

		while( true )
		{
			tempRawLongFileNames = extractLongFileNames( mappedClusterPtr );

			if( *( static_cast<char*>( mappedClusterPtr ) ) == 0 )
				break;

			tempRawDirEntry = *( reinterpret_cast<FatRawDirectoryEntry*>( mappedClusterPtr ) );

			dirEntries.push_back( DirectoryEntry( tempRawLongFileNames, tempRawDirEntry ) );

			mappedClusterPtr = reinterpret_cast<char*>(mappedClusterPtr) + sizeof( FatRawDirectoryEntry );
		}

		return dirEntries;
	}

	DirectoryEntry findNextFile( size_t folderCluster, const DirectoryEntry &prevFile = DirectoryEntry() )
	{
		std::vector<DirectoryEntry> dirEntries;

		dirEntries = getDirEntriesFromDirCluster( folderCluster );

		auto it = dirEntries.begin( );


		if( prevFile.type() != BAD_DIR_ENTRY )
			for( ; it->getName() == prevFile.getName(); ++it );

		for( ; it != dirEntries.end(); ++it )
		{
			if( it->type( ) == ARCHIVE )
				return *it;
		}

		return DirectoryEntry();
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
		const size_t lastClusterMagic = 0x0FFFFFF8;

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

public:
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

	void setPartitionPath( const std::string &partitionPath )
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

	void writeToEndOfCluster( const size_t clusterNo,
							  const size_t dataSize,
							  const char *data )
	{
		writeToCluster( clusterNo,
						clusterSize() - dataSize,
						dataSize,
						data );
	}

	void close()
	{
		mappedFileMngr.close();
	}
};

#endif