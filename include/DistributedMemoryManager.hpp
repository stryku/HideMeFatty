#ifndef _INCLUDE_DISTRIBUTEDMEMORYMAPPER_HPP_
#define _INCLUDE_DISTRIBUTEDMEMORYMAPPER_HPP_

#include <boost/random/mersenne_twister.hpp>

#include <vector>
#include <stdint.h> 
#include <fstream>

class DistributedMemoryManager
{
private:
	struct ChunkMetadata
	{
		char *ptr;
		size_t size;

		ChunkMetadata();
		ChunkMetadata( char *ptr, size_t size );
		~ChunkMetadata() {}
	};

	std::vector<ChunkMetadata> chunks;
	uint64_t totalSize;
	std::vector<uint64_t> shuffledArray;
    std::vector<uint64_t>::iterator shuffledIterator;
    char& at( uint64_t index );

public:
    DistributedMemoryManager() :
		totalSize( 0 )
    {}
    ~DistributedMemoryManager() {}

    void addMemoryChunk( char *ptr, size_t size );

    char& nextShuffledByteRef();

	void createShuffledArray( boost::random::mt19937 &rng );

	void clear();
};

#endif
