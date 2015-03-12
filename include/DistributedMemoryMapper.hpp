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

public:
	DistributedMemoryMapper() {}
	~DistributedMemoryMapper() {}

	void addMemoryChunk( char *ptr, size_t size )
	{
		chunks.push_back( ChunkMetadata( ptr, size ) );
		
		totalSize += size;
	}

	char& operator[]( size_t no )
	{
		size_t size = 0;

		

		for( const auto &chunk : chunks )
		{
			if( no < chunk.size )
				return chunk.ptr[no];

			no -= chunk.size;
		}
	}
};

#endif