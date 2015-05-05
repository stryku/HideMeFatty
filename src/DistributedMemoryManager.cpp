#include <DistributedMemoryManager.hpp>

DistributedMemoryManager::ChunkMetadata::ChunkMetadata() :
	ptr( nullptr ),
	size( 0 )
{}

DistributedMemoryManager::ChunkMetadata::ChunkMetadata( char *ptr,
													   size_t size ) :
	ptr( ptr ),
	size( size )
{}

void DistributedMemoryManager::addMemoryChunk( char *ptr, size_t size )
{
	chunks.push_back( ChunkMetadata( ptr, size ) );

	totalSize += size;
}

//todo change operator to method
char& DistributedMemoryManager::at( uint64_t index )
{
	for( const auto &chunk : chunks )
	{
        if( index < chunk.size )
            return chunk.ptr[index];

        index -= chunk.size;
	}
}

char& DistributedMemoryManager::nextShuffledByteRef()
{
    return at(*shuffledIterator++);
}

void DistributedMemoryManager::createShuffledArray( boost::random::mt19937 &rng )
{
	std::vector<bool> usedBytes( totalSize, false );

	shuffledArray.resize( totalSize );

	for( uint64_t i = 0; i < totalSize; ++i )
	{
		uint64_t ind;

		ind = rng() % totalSize;

		while( usedBytes[ind] )
		{
			++ind;
			ind %= totalSize;
		}

		shuffledArray[i] = ind;
		usedBytes[ind] = true;
	}

    shuffledIterator = shuffledArray.begin();
}

void DistributedMemoryManager::clear( )
{
	chunks.clear( );
	shuffledArray.clear( );
	totalSize = 0;
}
