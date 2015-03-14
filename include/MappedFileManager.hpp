#ifndef _INCLUDE_MAPPEDFILEMANAGER_
#define _INCLUDE_MAPPEDFILEMANAGER_

#include <string>
#include <set>
#include <boost\iostreams\device\mapped_file.hpp>
#include <boost\filesystem\operations.hpp>

typedef boost::iostreams::mapped_file MappedFile;

namespace fs = boost::filesystem;

class MappedFileManager
{
private:

	struct MappedChunk
	{
		intmax_t begin, end;
		bool mapped;

		MappedChunk();

		bool inside( const uintmax_t, const size_t ) const;

	} mappedChunkInfo;

	fs::path filePath;
	size_t allocationGranularity,
		mappingGranularity; //it is also mapped chunk size
	MappedFile mappedFile;

private:

	uintmax_t getOffsetForAllocGranularity( uintmax_t ) const;
	uintmax_t getOffsetForMapGranularity( uintmax_t ) const;
	uintmax_t getSizeForMapGranularity( const uintmax_t,
										const uintmax_t,
										size_t ) const;
	uintmax_t getSizeForAllocGranularity( const uintmax_t,
										  const uintmax_t,
										  size_t ) const;

	void remapChunk( uintmax_t, size_t, bool );

	char* getUserPtr( uintmax_t );

public:
	MappedFileManager();
	MappedFileManager( const size_t );
	~MappedFileManager();

	void setFilePath( const fs::path & );

	char* map( uintmax_t = 0, size_t = 0, bool = false );

	void close();
};

#endif