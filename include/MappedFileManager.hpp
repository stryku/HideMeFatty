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

	struct MappedChunk
	{
		intmax_t begin, end;
		bool mapped;

		MappedChunk() :
			mapped( false )
		{}

		bool inside( const uintmax_t offset, const size_t size ) const
		{
			return offset >= begin && offset + size <= end;
		}

	} mappedChunkInfo;

	std::string filePath;
	size_t allocationGranularity,
		mappingGranularity; //it is also mapped chunk size
	MappedFile mappedFile;

private:

	uintmax_t getOffsetForAllocGranularity( uintmax_t offset ) const
	{
		offset = offset / allocationGranularity * allocationGranularity;
		return offset;
	}

	uintmax_t getOffsetForMapGranularity( uintmax_t offset ) const
	{
		offset = offset / mappingGranularity * mappingGranularity;
		return offset;
	}

	uintmax_t getSizeForMapGranularity( const uintmax_t offset,
										  const uintmax_t preparedOffset,
										 size_t size ) const
	{
		
		size_t calculatedSize = mappingGranularity + size - ( preparedOffset + mappingGranularity - offset );

		return ( calculatedSize < mappingGranularity ) ? mappingGranularity : calculatedSize;
	}

	void remapChunk( uintmax_t startOffset, size_t sizeToMap )
	{
		uint64_t preparedOffset, preparedSize;

		preparedOffset = getOffsetForAllocGranularity( startOffset );
		preparedOffset = getOffsetForMapGranularity( preparedOffset );
		preparedSize = getSizeForMapGranularity( startOffset, preparedOffset, sizeToMap );

		if( !mappedChunkInfo.mapped || !mappedChunkInfo.inside( startOffset ) )

			mappedFile.open( filePath,
			std::ios_base::in | std::ios_base::out,
			preparedSize,
			preparedOffset );

		if( mappedFile.is_open() == false )
		{
			mappedChunkInfo.mapped = false;
			return;
		}

		mappedChunkInfo.begin = preparedOffset;
		mappedChunkInfo.end = preparedOffset + preparedSize;
	}

public:
	MappedFileManager( ) :
		mappingGranularity( 1073741824 ) // default: 1 gigabyte
	{
		allocationGranularity = mappedFile.alignment( );
	}
	MappedFileManager( const size_t _mappingGranularity ) :
		mappingGranularity( _mappingGranularity )
	{
		allocationGranularity = mappedFile.alignment( );
	}

	~MappedFileManager()
	{
		mappedFile.close();
	}

	void setFilePath( const std::string &pathToFile )
	{
		filePath = pathToFile;
	}

	char* map( uintmax_t startOffset = 0, size_t sizeToMap = 0 )
	{
		char *mappedPtr;
		uint64_t preparedOffset, preparedSize;

		if( !mappedChunkInfo.mapped || !mappedChunkInfo.inside( startOffset, sizeToMap ) )
			remapChunk( startOffset, sizeToMap );

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