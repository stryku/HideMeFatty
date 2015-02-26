#ifndef _INCLUDE_MAPPEDFILEMANAGER_
#define _INCLUDE_MAPPEDFILEMANAGER_

#ifdef _WIN32
	#include <Windows.h>
#endif //#ifdef _WIN32

#ifdef linux
	#include <unistd.h>
#endif //#ifdef linux


#include <string>
#include <set>
#include <boost\iostreams\device\mapped_file.hpp>

typedef boost::iostreams::mapped_file MappedFile;

class MappedFileManager
{
private:
	std::string partitionPath;
	size_t allocationGranularity;
	MappedFile mappedFile;

private:

	uint64_t getOffsetForGranularity( uint64_t offset ) const
	{
		offset = offset / allocationGranularity * allocationGranularity;
		return offset;
	}

	uint64_t getSizeForGranularity( const uint64_t offset,
										   const uint64_t preparedOffset,
										 size_t size ) const
	{
		size += offset - preparedOffset;
		return size;
	}


public:
	MappedFileManager() 
	{
		allocationGranularity = mappedFile.alignment( );
	}

	~MappedFileManager()
	{
		mappedFile.close();
	}

	void setPartitionPath( const std::string &pathToPartition )
	{
		partitionPath = pathToPartition;
	}

	char* map( uint64_t startOffset, size_t sizeToMap )
	{
		char *mappedPtr;
		uint64_t preparedOffset, preparedSize;

		preparedOffset = getOffsetForGranularity( startOffset );
		preparedSize = getSizeForGranularity( startOffset, preparedOffset, sizeToMap );

		mappedFile.open( partitionPath,
							 std::ios_base::in | std::ios_base::out,
							 preparedSize,
							 preparedOffset );


		if( mappedFile.is_open( ) == false )
			return nullptr;

		mappedPtr = mappedFile.data( );

		mappedPtr = mappedPtr+( startOffset - preparedOffset );

		return mappedPtr;
	}

	void unmap()
	{
		mappedFile.close();
	}
};

#endif