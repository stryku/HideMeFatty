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

//TODO
#include <easylogging++.h>
char& DistributedMemoryMapper::operator[]( uint64_t no )
{
	for( const auto &chunk : chunks )
	{
		if( no < chunk.size )
        {
            //LOG( INFO ) <<reinterpret_cast<u_int64_t>(chunk.ptr + no);
            vvv.push_back(chunk.ptr + no);
			return chunk.ptr[no];
        }

		no -= chunk.size;
	}
}

char& DistributedMemoryMapper::shuffled()
{
    yo++;
	return ( *this )[*shuffledIterator++];
}

//todo
#include <fstream>
void DistributedMemoryMapper::save()
{
    std::ofstream o("ssave");
    for(auto i : shuffledArray)
        o<<i<<"\n";

}

void DistributedMemoryMapper::createShuffledArray( boost::random::mt19937 &rng )
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

void DistributedMemoryMapper::clear( )
{
	chunks.clear( );
	shuffledArray.clear( );
	totalSize = 0;
}
