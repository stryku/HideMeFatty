#include <MappedFileManager.hpp>

MappedFileManager::MappedFileManager() :
	mappingGranularity( 1073741824 ) // default: 1 gigabyte
{
	allocationGranularity = mappedFile.alignment( );
}

MappedFileManager::MappedFileManager( const size_t _mappingGranularity ) :
	mappingGranularity( _mappingGranularity )
{
	allocationGranularity = mappedFile.alignment( );
}

MappedFileManager::~MappedFileManager( )
{
	mappedFile.close( );
}


MappedFileManager::MappedChunk::MappedChunk() :
	mapped( false )
{}

bool MappedFileManager::MappedChunk::inside( const uintmax_t offset, const size_t size ) const
{
	return offset >= begin && offset + size <= end;
}


uintmax_t MappedFileManager::getOffsetForAllocGranularity( uintmax_t offset ) const
{
	offset = offset / allocationGranularity * allocationGranularity;
	return offset;
}

uintmax_t MappedFileManager::getOffsetForMapGranularity( uintmax_t offset ) const
{
	offset = offset / mappingGranularity * mappingGranularity;
	return offset;
}

uintmax_t MappedFileManager::getSizeForMapGranularity( const uintmax_t offset,
									const uintmax_t preparedOffset,
									size_t size ) const
{

	size_t calculatedSize = mappingGranularity + size - ( preparedOffset + mappingGranularity - offset );

	return ( calculatedSize < mappingGranularity ) ? mappingGranularity : calculatedSize;
}

uintmax_t MappedFileManager::getSizeForAllocGranularity( const uintmax_t offset,
									  const uintmax_t preparedOffset,
									  size_t size ) const
{

	size_t calculatedSize = allocationGranularity + size - ( preparedOffset + allocationGranularity - offset );

	return ( calculatedSize < allocationGranularity ) ? allocationGranularity : calculatedSize;
}

void MappedFileManager::remapChunk( uintmax_t startOffset, size_t sizeToMap, bool hard )
{
	uint64_t preparedOffset, preparedSize;

	if( hard )
	{
		preparedOffset = getOffsetForAllocGranularity( startOffset );
		preparedSize = getSizeForAllocGranularity( startOffset, preparedOffset, sizeToMap );
	}
	else
	{
		preparedOffset = getOffsetForAllocGranularity( startOffset );
		preparedOffset = getOffsetForMapGranularity( preparedOffset );
		preparedSize = getSizeForMapGranularity( startOffset, preparedOffset, sizeToMap );
	}

	if( hard || !mappedChunkInfo.mapped || !mappedChunkInfo.inside( startOffset, preparedSize ) )
	{
		mappedFile.close( );

		mappedFile.open( filePath,
						 std::ios_base::in | std::ios_base::out,
						 preparedSize,
						 preparedOffset );
	}

	if( mappedFile.is_open( ) == false )
	{
		mappedChunkInfo.mapped = false;
		return;
	}

	mappedChunkInfo.begin = preparedOffset;
	mappedChunkInfo.end = preparedOffset + preparedSize;
	mappedChunkInfo.mapped = true;
}

char* MappedFileManager::getUserPtr( uintmax_t startOffset )
{
	char *preparedPtr;

	preparedPtr = mappedFile.data( );

	preparedPtr += startOffset - mappedChunkInfo.begin;

	return preparedPtr;
}

void MappedFileManager::setFilePath( const fs::path &pathToFile )
{
	filePath = pathToFile;
}

char* MappedFileManager::map( uintmax_t startOffset, size_t sizeToMap, bool hard )
{
	char *mappedPtr;
	uint64_t preparedOffset, preparedSize;

	if( hard || !mappedChunkInfo.mapped || !mappedChunkInfo.inside( startOffset, sizeToMap ) )
		remapChunk( startOffset, sizeToMap, hard );

	if( mappedFile.is_open( ) == false )
		return nullptr;

	mappedPtr = getUserPtr( startOffset );

	return mappedPtr;
}

void MappedFileManager::close( )
{
	mappedFile.close( );
}