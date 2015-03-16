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


uintmax_t MappedFileManager::getOffsetForGranularity( uintmax_t offset, const size_t granularity ) const
{
	offset = offset / granularity * granularity;
	return offset;
}

uintmax_t MappedFileManager::getSizeForGranularity( const uintmax_t offset,
													const uintmax_t preparedOffset,
													size_t size,
													const size_t granularity) const
{

	size_t calculatedSize = granularity + size - ( preparedOffset + granularity - offset );

	return ( calculatedSize < granularity ) ? granularity : calculatedSize;
}

void MappedFileManager::remapChunk( uintmax_t startOffset, size_t sizeToMap, bool hard )
{
	uint64_t preparedOffset, preparedSize;

	LOG( INFO ) << "Remapping chunk";

	if( hard )
	{
		preparedOffset = getOffsetForGranularity( startOffset, allocationGranularity );
		preparedSize = getSizeForGranularity( startOffset, preparedOffset, sizeToMap, allocationGranularity );
		LOG( INFO ) << "Hard remapping. Prepared offset = " << preparedOffset << ", prepared size: " << preparedSize;
	}
	else
	{
		preparedOffset = getOffsetForGranularity( startOffset, allocationGranularity );
		preparedOffset = getOffsetForGranularity( preparedOffset, mappingGranularity );
		preparedSize = getSizeForGranularity( startOffset, preparedOffset, sizeToMap, mappingGranularity );
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

	LOG( INFO ) << "Preparing mapped ptr to what user wanted. Oryginally mapped at: 0x" \
		<< std::hex << static_cast<uintptr_t>( reinterpret_cast<uint32_t>( mappedFile.data() ) );

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

	LOG( INFO ) << "Returning 0x" << std::hex \
		<< static_cast<uintptr_t>( reinterpret_cast<uint32_t>( mappedPtr ) );

	return mappedPtr;
}

void MappedFileManager::close( )
{
	LOG( INFO ) << "Closing";
	mappedFile.close( );
}