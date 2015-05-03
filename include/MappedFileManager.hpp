#ifndef _INCLUDE_MAPPEDFILEMANAGER_
#define _INCLUDE_MAPPEDFILEMANAGER_

#include <string>
#include <set>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem/operations.hpp>

typedef boost::iostreams::mapped_file MappedFile;
typedef boost::iostreams::mapped_file::size_type boostSize_t;

namespace fs = boost::filesystem;

class MappedFileManager
{
private:

	struct MappedChunk
	{
		uint64_t begin, end;
		bool mapped;

		MappedChunk();

		bool inside( const uint64_t offset, const uint64_t size ) const;

	} mappedChunkInfo;

	fs::path filePath;
	size_t allocationGranularity,
		mappingGranularity; //it is also mapped chunk size
	MappedFile mappedFile;

private:

	uint64_t getOffsetForGranularity( uint64_t offset, const size_t granularity ) const;
	uint64_t getSizeForGranularity( const uint64_t offset,
									 const uint64_t preparedOffset,
									 uint64_t size,
									 const size_t granularity) const;

	void remapChunk( uint64_t startOffset, uint64_t sizeToMap, bool hard );

	char* getUserPtr( uint64_t startOffset );

public:
	MappedFileManager();
	MappedFileManager( const size_t _mappingGranularity );
	~MappedFileManager();

	void setFilePath( const fs::path &pathToFile );

	char* map( uint64_t startOffset = 0, uint64_t sizeToMap = 0, bool hard = false );

	void close();

	friend std::ostream& operator<<( std::ostream &, const MappedChunk & );
};

#endif
