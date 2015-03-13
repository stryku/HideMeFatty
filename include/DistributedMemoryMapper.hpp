#ifndef _INCLUDE_DISTRIBUTEDMEMORYMAPPER_HPP_
#define _INCLUDE_DISTRIBUTEDMEMORYMAPPER_HPP_

#include <vector>

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

	template <class T>
	void createShuffledArray( T &rng );

	void clear();
};

#endif