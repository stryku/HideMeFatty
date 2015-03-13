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

		ChunkMetadata() :
			ptr( nullptr ),
			size( 0 )
		{}

		ChunkMetadata( char *ptr, size_t size ) :
			ptr( ptr ),
			size( size )
		{}
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

	void addMemoryChunk( char *ptr, size_t size )
	{
		chunks.push_back( ChunkMetadata( ptr, size ) );
		
		totalSize += size;
	}

	char& operator[]( uintmax_t no )
	{
		uintmax_t size = 0;

		for( const auto &chunk : chunks )
		{
			if( no < chunk.size )
				return chunk.ptr[no];

			no -= chunk.size;
		}
	}

	char& shuffled()
	{
		return (*this)[*shuffledIterator++];
	}

	template <class T>
	void createShuffledArray( T &rng )
	{
		std::vector<bool> usedBytes( totalSize, false );

		shuffledArray.resize( totalSize );

		for( uintmax_t i = 0; i < totalSize; ++i )
		{
			uintmax_t ind;

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

	void clear()
	{
		chunks.clear();
		shuffledArray.clear();
		totalSize = 0;
	}
};

#endif