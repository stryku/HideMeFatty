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
		ChunkMetadata( char *, size_t  );
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

	void addMemoryChunk( char *, size_t  );

	char& operator[]( uintmax_t );

	char& shuffled();

	void createShuffledArray( boost::random::mt19937 & );

	void clear();
};

#endif