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
	inline std::wstring getPartOfName( const FatRawLongFileName & ) const;

	inline std::wstring extractExtension( const FatRawDirectoryEntry & ) const;
	inline std::wstring extractShortName( const FatRawDirectoryEntry & ) const;
	inline std::wstring extractName( const std::vector<FatRawLongFileName> &,
									 const FatRawDirectoryEntry & ) const;
	inline Date extractDate( const unsigned short int , const unsigned short int  ) const;
	inline bool extractIfDeleted( const FatRawDirectoryEntry & ) const;
	inline size_t extractCluster( const FatRawDirectoryEntry & ) const;

public:
	DirectoryEntry();
	DirectoryEntry( const std::vector<FatRawLongFileName> &,
					const FatRawDirectoryEntry & );
	~DirectoryEntry() {}

	void assign( const std::vector<FatRawLongFileName> &, const FatRawDirectoryEntry & );

	EDirEntryType type() const;

	std::wstring getName() const;
	uint64_t getFileSize() const;
	size_t getCluster() const;

	void setCluster( size_t cluster );
	bool operator==( const DirectoryEntry & ) const;
};

#endif