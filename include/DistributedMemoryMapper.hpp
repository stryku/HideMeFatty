#ifndef _INCLUDE_DISTRIBUTEDMEMORYMAPPER_HPP_
#define _INCLUDE_DISTRIBUTEDMEMORYMAPPER_HPP_

#include <boost/random/mersenne_twister.hpp>

#include <vector>
#include <stdint.h> 

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

	std::vector<ChunkMetadata> chunks;
	size_t totalSize;
	std::vector<uintmax_t> shuffledArray;
	std::vector<uintmax_t>::iterator shuffledIterator;
	

public:
	DistributedMemoryMapper() :
		totalSize( 0 )
	{}
	~DistributedMemoryMapper() {}

	void addMemoryChunk( char *ptr, size_t size );

	char& operator[]( uintmax_t no );

	char& shuffled();

	void createShuffledArray( boost::random::mt19937 &rng );

	void clear();
};

#endif