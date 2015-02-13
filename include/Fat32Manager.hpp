#ifndef _INCLUDE_FAT32MANAGER_
#define _INCLUDE_FAT32MANAGER_

#include <Windows.h>
#include <string>

#include <FatStructs.h>

class Fat32Manager
{
private:
	std::string _partitionPath;
	char *_viewOfFile;
	FatBS *_bootSector;
	Fat32ExtBS *_fat32ExtBS;
	FatInfo fatInfo;
	bool fatInfoLoaded;

	//private methods
private:

	bool createMapViewOfFile( )
	{
		HANDLE createFileHandle,
			createFileMappingHandle;
		size_t fileSize;

		if( _partitionPath == "" )
			return false;

		createFileHandle = CreateFile( std::wstring( _partitionPath.begin( ), _partitionPath.end( ) ).c_str( ),
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

		_viewOfFile = reinterpret_cast<char*>( MapViewOfFile( createFileMappingHandle,
			FILE_MAP_ALL_ACCESS,
			NULL,
			NULL,
			NULL ) );

		return true;
	}

	bool loadBootSector()
	{
		if( _viewOfFile == nullptr )
		{
			if( !createMapViewOfFile() )
				return false;
		}

		_bootSector = reinterpret_cast<FatBS*>( _viewOfFile );

		return true;
	}

	void loadFatInfo()
	{
		fatInfo.total_sectors = ( _bootSector->total_sectors_16 == 0 ) ? _bootSector->total_sectors_32 : _bootSector->total_sectors_16;
		fatInfo.fat_size = ( _bootSector->table_size_16 == 0 ) ? _bootSector->total_sectors_32 : _bootSector->table_size_16;
		fatInfo.root_dir_sectors = ( ( _bootSector->root_entry_count * 32 ) + ( _bootSector->bytes_per_sector - 1 ) ) / _bootSector->bytes_per_sector;
		fatInfo.first_data_sector = _bootSector->reserved_sector_count + ( _bootSector->table_count * fatInfo.fat_size ) + fatInfo.root_dir_sectors;
		fatInfo.first_fat_sector = _bootSector->reserved_sector_count;
		fatInfo.data_sectors = fatInfo.total_sectors - ( _bootSector->reserved_sector_count + ( _bootSector->table_count * fatInfo.fat_size ) + fatInfo.root_dir_sectors );
		fatInfo.total_clusters = fatInfo.data_sectors / _bootSector->sectors_per_cluster;

		fatInfoLoaded = true;
	}

	bool isValidFat32()
	{
		if( _bootSector == nullptr )
		{
			if( !loadBootSector( ) )
				return false;
		}

		if( fatInfoLoaded == false )
			loadFatInfo();

		return getFatType() == FAT32;
	}

public:
	Fat32Manager() :
		_viewOfFile( nullptr ),
		_bootSector( nullptr ),
		fatInfoLoaded( false )
	{}
	Fat32Manager( const std::string &partitionPath ) :
		_partitionPath( _partitionPath ),
		_viewOfFile( nullptr ),
		_bootSector( nullptr ),
		fatInfoLoaded( false )
	{}
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
};

#endif