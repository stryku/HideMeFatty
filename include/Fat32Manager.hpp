#ifndef _INCLUDE_FAT32MANAGER_
#define _INCLUDE_FAT32MANAGER_

#include <memory>

#include <FatStructs.h>
#include <MappedFileManager.hpp>

class Fat32Manager
{
private:
	static const size_t bootSectorSize = 512;
	
	FatBS bootSector;
	Fat32ExtBS fat32ExtBS;
	FatInfo fatInfo;
	bool fatInfoLoaded, bootSectorLoaded;
	MappedFileManager mappedFileMngr;

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

		if( !mappedFileMngr.good() )
			return false;

		mappedPtr = static_cast<FatBS*>( mappedFileMngr.map( 0, bootSectorSize ) );

		if( mappedPtr == nullptr )
			return false;

		bootSector = *mappedPtr;

		loadFat32ExtBS();

		bootSectorLoaded = true;

		mappedFileMngr.unmap( mappedPtr );

		return true;
	}

	void loadFatInfo()
	{
		if( fatInfoLoaded )
			return;

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

	unsigned char* loadCluster( size_t clusterNo )
	{
		void *mappedPtr;
		mappedPtr = mappedFileMngr.map( getClusterStartOffset( clusterNo ),
										clusterSize() );
		return static_cast<unsigned char*>( mappedPtr );
	}
	
public:
	Fat32Manager() :
		fatInfoLoaded( false ),
		bootSectorLoaded( false )
	{}

	Fat32Manager( const std::string &partitionPath ) :
		fatInfoLoaded( false ),
		bootSectorLoaded( false )
	{
		mappedFileMngr.setPartitionPath( partitionPath );
		mappedFileMngr.init();
	}

	~Fat32Manager() {}

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
		unsigned char *clusterPtr;
		/*FatDirectoryEntry *dirEntry;

		clusterPtr = loadCluster( fat32ExtBS.root_cluster );
		dirEntry = reinterpret_cast<FatDirectoryEntry*>( clusterPtr );*/


	}
};

#endif