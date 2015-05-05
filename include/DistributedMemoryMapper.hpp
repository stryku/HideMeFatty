#ifndef _INCLUDE_DISTRIBUTEDMEMORYMAPPER_HPP_
#define _INCLUDE_DISTRIBUTEDMEMORYMAPPER_HPP_

#include <boost/random/mersenne_twister.hpp>

#include <vector>
#include <stdint.h> 
#include <fstream>

class DistributedMemoryMapper
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

    std::vector<char*> vvv; //todo
	std::vector<ChunkMetadata> chunks;
	uint64_t totalSize;
	std::vector<uint64_t> shuffledArray;
	std::vector<uint64_t>::iterator shuffledIterator;
	
    size_t yo; //todo

public:
	DistributedMemoryMapper() :
		totalSize( 0 )
    {}
    ~DistributedMemoryMapper() {}

	void addMemoryChunk( char *ptr, size_t size );

	char& operator[]( uint64_t no );

	char& shuffled();

	void createShuffledArray( boost::random::mt19937 &rng );

	void clear();
};

#endif
