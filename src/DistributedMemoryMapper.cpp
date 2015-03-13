#include <DistributedMemoryMapper.hpp>

DistributedMemoryMapper::ChunkMetadata::ChunkMetadata() :
	ptr( nullptr ),
	size( 0 )
{}

DistributedMemoryMapper::ChunkMetadata::ChunkMetadata( char *ptr, 
													   size_t size ) :
	ptr( ptr ),
	size( size )
{}

void DistributedMemoryMapper::addMemoryChunk( char *ptr, size_t size )
{
	chunks.push_back( ChunkMetadata( ptr, size ) );

	totalSize += size;
}

char& DistributedMemoryMapper::operator[]( uintmax_t no )
{
	uintmax_t size = 0;

	for( const auto &chunk : chunks )
	{
		if( no < chunk.size )
			return chunk.ptr[no];

		no -= chunk.size;
	}
}

char& DistributedMemoryMapper::shuffled( )
{
	return ( *this )[*shuffledIterator++];
}

template <class T>
void DistributedMemoryMapper::createShuffledArray( T &rng )
{
	std::vector<bool> usedBytes( totalSize, false );

	shuffledArray.resize( totalSize );

	for( uintmax_t i = 0; i < totalSize; ++i )
	{
		uintmax_t ind;

		ind = rng( ) % totalSize;

		while( usedBytes[ind] )
		{
			++ind;
			ind %= totalSize;
		}

		shuffledArray[i] = ind;
		usedBytes[ind] = true;
	}

	shuffledIterator = shuffledArray.begin( );
}

void DistributedMemoryMapper::clear( )
{
	chunks.clear( );
	shuffledArray.clear( );
	totalSize = 0;
}