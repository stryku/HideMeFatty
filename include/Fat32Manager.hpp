#ifndef _INCLUDE_FAT32MANAGER_
#define _INCLUDE_FAT32MANAGER_

#include <Windows.h>
#include <string>
#include <memory>

#include <FatStructs.h>

class Fat32Manager
{
private:
	static const size_t bootSectorSize = 512;

	std::string partitionPath;
	HANDLE createFileHandle,
		createFileMappingHandle;
	char *viewOfFile;
	FatBS bootSector;
	Fat32ExtBS fat32ExtBS;
	FatInfo fatInfo;
	bool fatInfoLoaded, bootSectorLoaded;
	SYSTEM_INFO systemInfo;

	//private methods
private:

	bool createMapViewOfFile( )
	{
		
		size_t fileSize;

		if( partitionPath == "" )
			return false;

		createFileHandle = CreateFile( std::wstring( partitionPath.begin( ), partitionPath.end( ) ).c_str( ),
									   GENERIC_READ | GENERIC_WRITE,
									   NULL,
									   NULL,
									   OPEN_EXISTING,
									   FILE_ATTRIBUTE_NORMAL,
									   NULL );

		if( createFileHandle == INVALID_HANDLE_VALUE )
			return false;

		createFileMappingHandle = CreateFileMapping( createFileHandle,
													 NULL,
													 PAGE_READWRITE,
													 NULL,
													 NULL,
													 NULL );

		if( createFileMappingHandle == NULL )
			return false;

		viewOfFile = reinterpret_cast<char*>( MapViewOfFile( createFileMappingHandle,
			FILE_MAP_ALL_ACCESS,
			NULL,
			NULL,
			bootSectorSize ) );

		if( viewOfFile == nullptr )
			return false;

		return true;
	}

	void loadFat32ExtBS()
	{
		fat32ExtBS = *( reinterpret_cast<Fat32ExtBS*>( &bootSector.extended_section ) );
	}

	bool loadBootSector()
	{
		if( viewOfFile == nullptr )
		{
			if( !createMapViewOfFile() )
				return false;
		}

		bootSector = *( reinterpret_cast<FatBS*>( viewOfFile ) );

		loadFat32ExtBS();

		bootSectorLoaded = true;

		return true;
	}

	void loadFatInfo()
	{
		fatInfo.total_sectors = ( bootSector.total_sectors_16 == 0 ) ? bootSector.total_sectors_32 : bootSector.total_sectors_16;
		fatInfo.fat_size = ( bootSector.table_size_16 == 0 ) ? bootSector.total_sectors_32 : bootSector.table_size_16;
		fatInfo.root_dir_sectors = ( ( bootSector.root_entry_count * 32 ) + ( bootSector.bytes_per_sector - 1 ) ) / bootSector.bytes_per_sector;
		fatInfo.first_data_sector = bootSector.reserved_sector_count + ( bootSector.table_count * fatInfo.fat_size ) + fatInfo.root_dir_sectors;
		fatInfo.first_fat_sector = bootSector.reserved_sector_count;
		fatInfo.data_sectors = fatInfo.total_sectors - ( bootSector.reserved_sector_count + ( bootSector.table_count * fatInfo.fat_size ) + fatInfo.root_dir_sectors );
		fatInfo.total_clusters = fatInfo.data_sectors / bootSector.sectors_per_cluster;

		fatInfoLoaded = true;
	}

	inline size_t clusterSize()
	{ 
		return bootSector.bytes_per_sector * bootSector.sectors_per_cluster;
	}
	inline size_t getClusterStartOffset( size_t clusterNo )
	{
		return clusterNo * clusterSize();
	}

	inline size_t prepareOffsetForGranularity( size_t offset )
	{
		offset = offset / systemInfo.dwAllocationGranularity * systemInfo.dwAllocationGranularity;
		return offset;
	}

	char* mapCluster( size_t clusterNo )
	{
		size_t clusterOffset, preparedOffset;
		DWORD fileOffsetHigh, fileOffsetLow;
		
		clusterOffset = getClusterStartOffset( clusterNo );
		preparedOffset = prepareOffsetForGranularity( clusterOffset );

		fileOffsetHigh = preparedOffset >> 32; // high 32 bit
		fileOffsetLow = preparedOffset & 0xffffffff; // low 32 bit

		viewOfFile = reinterpret_cast<char*>( MapViewOfFile( createFileMappingHandle,
			FILE_MAP_ALL_ACCESS,
			fileOffsetHigh,
			fileOffsetLow,
			bootSectorSize ) );
	}

public:
	Fat32Manager() :
		viewOfFile( nullptr ),
		fatInfoLoaded( false ),
		bootSectorLoaded( false )
	{
		GetSystemInfo( &systemInfo );
	}
	Fat32Manager( const std::string &partitionPath ) :
		partitionPath( partitionPath ),
		viewOfFile( nullptr ),
		fatInfoLoaded( false ),
		bootSectorLoaded( false )
	{
		GetSystemInfo( &systemInfo );
	}
	~Fat32Manager() 
	{

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
		if( bootSectorLoaded == false )
		{
			if( !loadBootSector( ) )
				return false;
		}

		if( fatInfoLoaded == false )
			loadFatInfo( );

		return getFatType( ) == FAT32;
	}

	void printFiles()
	{
		char *clusterPtr;
		FatDirectoryEntry *dirEntry;

		clusterPtr = readCluster( fat32ExtBS.root_cluster );
		dirEntry = reinterpret_cast<FatDirectoryEntry*>( clusterPtr );

	}
};

#endif