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

bool MappedFileManager::MappedChunk::inside( const uint64_t offset, const uint64_t size ) const
{
	return offset >= begin && offset + size <= end;
}


uint64_t MappedFileManager::getOffsetForGranularity( uint64_t offset, const size_t granularity ) const
{
	offset = offset / granularity * granularity;
	return offset;
}

uint64_t MappedFileManager::getSizeForGranularity( const uint64_t offset,
													const uint64_t preparedOffset,
													uint64_t size,
													const size_t granularity) const
{

	uint64_t calculatedSize = granularity + size - ( preparedOffset + granularity - offset );

	return ( calculatedSize < granularity ) ? granularity : calculatedSize;
}

void MappedFileManager::remapChunk( uint64_t startOffset, uint64_t sizeToMap, bool hard )
{
	uint64_t preparedOffset, preparedSize;

	if( !fs::exists( filePath ) )
		return;

	if( hard )
	{
		preparedOffset = getOffsetForGranularity( startOffset, allocationGranularity );
		preparedSize = getSizeForGranularity( startOffset, preparedOffset, sizeToMap, allocationGranularity );
	}
	else
    {
		preparedOffset = getOffsetForGranularity( startOffset, allocationGranularity );
		preparedOffset = getOffsetForGranularity( preparedOffset, mappingGranularity );
		preparedSize = getSizeForGranularity( startOffset, preparedOffset, sizeToMap, mappingGranularity );
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
    mappedChunkInfo.end = preparedOffset + preparedSize;//preparedSize;
	mappedChunkInfo.mapped = true;
}

std::ostream& operator<<( std::ostream &out, const MappedFileManager::MappedChunk &mc )
{
	out << "Begin = " << mc.begin\
		<< "End = " << mc.end\
		<< "Mapped = " << mc.mapped;

	return out;
}

char* MappedFileManager::getUserPtr( uint64_t startOffset )
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

char* MappedFileManager::map( uint64_t startOffset, uint64_t sizeToMap, bool hard )
{
	char *mappedPtr;

	if( hard || !mappedChunkInfo.mapped || !mappedChunkInfo.inside( startOffset, sizeToMap ) )
        remapChunk( startOffset, sizeToMap, hard );

	if( mappedFile.is_open() == false )
		return nullptr;

    mappedPtr = getUserPtr( startOffset );

	return mappedPtr;
}

void MappedFileManager::close( )
{
	mappedFile.close( );
}
