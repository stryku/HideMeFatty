#ifndef _INCLUDE_MAPPEDFILEMANAGER_
#define _INCLUDE_MAPPEDFILEMANAGER_

#include <string>
#include <set>

#include <boost\iostreams\device\mapped_file.hpp>
#include <boost\filesystem\operations.hpp>

typedef boost::iostreams::mapped_file MappedFile;
typedef boost::iostreams::mapped_file::size_type boostSize_t;

namespace fs = boost::filesystem;

class MappedFileManager
{
private:

	struct MappedChunk
	{
		uintmax_t begin, end;
		bool mapped;

		MappedChunk();

		bool inside( const uintmax_t offset, const uintmax_t size ) const;

	} mappedChunkInfo;

	fs::path filePath;
	size_t allocationGranularity,
		mappingGranularity; //it is also mapped chunk size
	MappedFile mappedFile;

private:

	uintmax_t getOffsetForGranularity( uintmax_t offset, const size_t granularity ) const;
	uintmax_t getSizeForGranularity( const uintmax_t offset,
									 const uintmax_t preparedOffset,
									 uintmax_t size,
									 const size_t granularity) const;

	void remapChunk( uintmax_t startOffset, uintmax_t sizeToMap, bool hard );

	char* getUserPtr( uintmax_t startOffset );

public: 
	MappedFileManager();
	MappedFileManager( const size_t _mappingGranularity );
	~MappedFileManager();

	void setFilePath( const fs::path &pathToFile );

	char* map( uintmax_t startOffset = 0, uintmax_t sizeToMap = 0, bool hard = false );

	void close();

	friend std::ostream& operator<<( std::ostream &, const MappedChunk & );
};

#endif 