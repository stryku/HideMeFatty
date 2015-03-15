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

	LOG( INFO ) << "Remapping chunk";

	if( hard )
	{
		preparedOffset = getOffsetForAllocGranularity( startOffset );
		preparedSize = getSizeForAllocGranularity( startOffset, preparedOffset, sizeToMap );
		LOG( INFO ) << "Hard remapping. Prepared offset = " << preparedOffset << ", prepared size: " << preparedSize;
	}
	else
	{
		preparedOffset = getOffsetForAllocGranularity( startOffset );
		preparedOffset = getOffsetForMapGranularity( preparedOffset );
		preparedSize = getSizeForMapGranularity( startOffset, preparedOffset, sizeToMap );
		LOG( INFO ) << "Soft remapping. Prepared offset = " << preparedOffset << ", prepared size: " << preparedSize;
	}

	mappedFile.close();

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
	mappedChunkInfo.mapped = true;

	LOG( INFO ) << "Mapped chunk info: " << mappedChunkInfo;
}

std::ostream& operator<<( std::ostream &out, const MappedFileManager::MappedChunk &mc )
{
	out << "Begin = " << mc.begin\
		<< "End = " << mc.end\
		<< "Mapped = " << mc.mapped;

	return out;
}

char* MappedFileManager::getUserPtr( uintmax_t startOffset )
{
	char *preparedPtr;

	LOG( INFO ) << "Preparing mapped ptr to what user wanted. Oryginally mapped at: 0x" << std::hex << mappedFile.data();

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

	LOG( INFO ) << "Mapping. Start offset = " << startOffset \
		<< ", size to map = " << sizeToMap \
		<< ", hard = " << std::boolalpha << hard \
		<< ", already mapped: " << std::boolalpha<<mappedChunkInfo.mapped;

	if( hard || !mappedChunkInfo.mapped || !mappedChunkInfo.inside( startOffset, sizeToMap ) )
	{
		LOG( INFO ) << "Need to remap";
		remapChunk( startOffset, sizeToMap, hard );
	}

	if( mappedFile.is_open() == false )
	{
		LOG( INFO ) << "Mapped file isn't open. Returning nullptr";
		return nullptr;
	}

	mappedPtr = getUserPtr( startOffset );

	LOG( INFO ) << "Returning 0x" << std::hex << mappedPtr;
	return mappedPtr;
}

void MappedFileManager::close( )
{
	LOG( INFO ) << "Closing";
	mappedFile.close( );
}