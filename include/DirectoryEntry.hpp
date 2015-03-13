#ifndef _INCLUDE_DIRECTORYENTRY_
#define _INCLUDE_DIRECTORYENTRY_

#include <vector>
#include <memory>
#include <string>

#include <FatStructs.h>
#include <Structs.h>

class DirectoryEntry
{
private:
	static const unsigned char DELETED_MAGIC = 0xE5;
	static const char END_OF_NAME = 0x20;

	std::wstring name;
	size_t fileSize;
	unsigned char attributes;
	size_t cluster;
	Date creationDate, lastModificationDate;
	bool deleted;

private:
	inline std::wstring getPartOfName( const FatRawLongFileName &longFileName ) const;

	inline std::wstring extractExtension( const FatRawDirectoryEntry &rawDirEntry ) const;
	inline std::wstring extractShortName( const FatRawDirectoryEntry &rawDirEntry ) const;
	inline std::wstring extractName( const std::vector<FatRawLongFileName> &longFileNames,
									 const FatRawDirectoryEntry &rawDirEntry ) const;
	inline Date extractDate( const unsigned short int time, const unsigned short int date ) const;
	inline bool extractIfDeleted( const FatRawDirectoryEntry &rawDirEntry ) const;
	inline size_t extractCluster( const FatRawDirectoryEntry &rawDirEntry ) const;

public:
	DirectoryEntry();
	DirectoryEntry( const std::vector<FatRawLongFileName> &longFileNames,
					const FatRawDirectoryEntry &rawDirEntry );
	~DirectoryEntry() {}

	void assign( const std::vector<FatRawLongFileName> &longFileNames, const FatRawDirectoryEntry &rawDirEntry );

	void print( std::wostream &ostream ) const;

	EDirEntryType type() const;

	std::wstring getName() const;
	uint64_t getFileSize() const;
	size_t getCluster() const;

	void setCluster( size_t cluster );
	bool operator==( const DirectoryEntry &de ) const;
};

#endif