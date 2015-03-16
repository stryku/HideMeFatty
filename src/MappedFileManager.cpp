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

bool MappedFileManager::MappedChunk::inside( const uintmax_t offset, const uintmax_t size ) const
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
													uintmax_t size,
													const size_t granularity) const
{

	uintmax_t calculatedSize = granularity + size - ( preparedOffset + granularity - offset );

	return ( calculatedSize < granularity ) ? granularity : calculatedSize;
}

void MappedFileManager::remapChunk( uintmax_t startOffset, uintmax_t sizeToMap, bool hard )
{
	uintmax_t preparedOffset, preparedSize;

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
	mappedChunkInfo.end = preparedOffset + preparedSize;
	mappedChunkInfo.mapped = true;
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

	preparedPtr = mappedFile.data( );

	preparedPtr += startOffset - mappedChunkInfo.begin;

	return preparedPtr;
}

void MappedFileManager::setFilePath( const fs::path &pathToFile )
{
	filePath = pathToFile;
}

char* MappedFileManager::map( uintmax_t startOffset, uintmax_t sizeToMap, bool hard )
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