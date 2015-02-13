#ifndef _INCLUDE_FAT32MANAGER_
#define _INCLUDE_FAT32MANAGER_

#include <Windows.h>
#include <string>

class Fat32Manager
{
private:
	std::string _partitionPath;
	char *_viewOfFilePtr;

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

		_viewOfFilePtr = reinterpret_cast<char*>( MapViewOfFile( createFileMappingHandle,
			FILE_MAP_ALL_ACCESS,
			NULL,
			NULL,
			NULL ) );

		return true;
	}

	bool isValidFat32()
	{

	}

public:
	Fat32Manager() :
		_viewOfFilePtr( nullptr )
	{}
	Fat32Manager( const std::string &partitionPath ) :
		_partitionPath( _partitionPath ),
		_viewOfFilePtr( nullptr )
	{}
	~Fat32Manager() {}
};

#endif