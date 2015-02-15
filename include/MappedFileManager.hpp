#ifndef _INCLUDE_MAPPEDFILEMANAGER_
#define _INCLUDE_MAPPEDFILEMANAGER_

#include <string>
#include <Windows.h>
#include <set>

class MappedFileManager
{
private:
	std::string partitionPath;
	HANDLE createFileHandle,
	createFileMappingHandle;
	char *viewOfFile;
	SYSTEM_INFO systemInfo;
	std::set<LPVOID> mappedPointers;

private:
	void createFileHandles()
	{
		if( partitionPath == "" )
			return;

		createFileHandle = CreateFile( std::wstring( partitionPath.begin(), partitionPath.end() ).c_str(),
									   GENERIC_READ | GENERIC_WRITE,
									   NULL,
									   NULL,
									   OPEN_EXISTING,
									   FILE_ATTRIBUTE_NORMAL,
									   NULL );

		if( createFileHandle == INVALID_HANDLE_VALUE )
			return;

		createFileMappingHandle = CreateFileMapping( createFileHandle,
													 NULL,
													 PAGE_READWRITE,
													 NULL,
													 NULL,
													 NULL );
	}

	inline uint64_t getOffsetForGranularity( uint64_t offset ) const
	{
		offset = offset / systemInfo.dwAllocationGranularity * systemInfo.dwAllocationGranularity;
		return offset;
	}

	inline uint64_t getSizeForGranularity( const uint64_t offset,
										   const uint64_t preparedOffset,
										 size_t size ) const
	{
		size += offset - preparedOffset;
		return size;
	}

public:
	MappedFileManager() :
		createFileHandle( INVALID_HANDLE_VALUE ),
		createFileMappingHandle( NULL )
	{
		GetSystemInfo( &systemInfo );
	}
	~MappedFileManager() 
	{
		for( auto &it = mappedPointers.begin();
			 it != mappedPointers.end();
			 it = mappedPointers.begin() )
		{
			unmap( *it );
		}
		

		if( createFileMappingHandle != NULL)
			CloseHandle( createFileMappingHandle );

		if( createFileHandle != INVALID_HANDLE_VALUE)
			CloseHandle( createFileHandle );
	}

	void setPartitionPath( const std::string &pathToPartition )
	{
		partitionPath = pathToPartition;
	}

	void init()
	{
		createFileHandles();
	}

	bool good() const
	{
		return	createFileHandle != INVALID_HANDLE_VALUE &&
				createFileMappingHandle != NULL;
	}

	void* map( uint64_t startOffset, size_t sizeToMap )
	{
		void *mappedPtr;
		uint64_t preparedOffset, preparedSize;
		DWORD offsetHigh, offsetLow;

		preparedOffset = getOffsetForGranularity( startOffset );
		preparedSize = getSizeForGranularity( startOffset, preparedOffset, sizeToMap );

		offsetHigh = preparedOffset >> 32;
		offsetLow = preparedOffset & 0xffffffff;

		mappedPtr = MapViewOfFile( createFileMappingHandle,
								   FILE_MAP_ALL_ACCESS,
								   offsetHigh,
								   offsetLow,
								   preparedSize );

		mappedPointers.insert( mappedPtr );

		if( mappedPtr == nullptr )
			return nullptr;

		mappedPtr = reinterpret_cast<char*>(mappedPtr)+( startOffset - preparedOffset );

		return mappedPtr;
	}

	void unmap( void *ptr )
	{
		if( ptr != nullptr )
			UnmapViewOfFile( ptr );

		mappedPointers.erase( ptr );
	}
};

#endif