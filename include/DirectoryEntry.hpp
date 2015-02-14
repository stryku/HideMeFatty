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
	static const unsigned int DELETED_MAGIC = 0xE5;

	std::wstring name;
	size_t fileSize;
	unsigned char attributes;
	size_t cluster;
	Date creationDate, lastModificationDate;
	bool deleted;


private:
	inline std::wstring getPartOfName( const FatRawLongFileName &longFileName ) const
	{
		std::wstring namePart;

		for( size_t i = 0; i < 5; ++i )
		{
			if( longFileName.firtsChars[i] == '\0' )
				return namePart;

			namePart += longFileName.firtsChars[i];
		}

		for( size_t i = 0; i < 6; ++i )
		{
			if( longFileName.firtsChars[i] == '\0' )
				return namePart;

			namePart += longFileName.firtsChars[i];
		}

		for( size_t i = 0; i < 2; ++i )
		{
			if( longFileName.firtsChars[i] == '\0' )
				return namePart;

			namePart += longFileName.firtsChars[i];
		}

		return namePart;
	}

	inline std::wstring extractName( const std::vector<FatRawLongFileName> &longFileNames, const FatRawDirectoryEntry &rawDirEntry ) const
	{
		std::wstring tempName;

		for( size_t i = 0; i < 8; ++i )
		{
			if( rawDirEntry.fileName[i] == '\0' )
				break;

			tempName += rawDirEntry.fileName[i];
		}

		for( const auto &i : longFileNames )
		{
			tempName += getPartOfName( i );
		}

		tempName += '.';

		for( size_t i = 8; i < 11; ++i )
		{
			if( rawDirEntry.fileName[i] == '\0' )
				break;

			tempName += rawDirEntry.fileName[i];
		}
	}
	inline Date extractDate( const unsigned short int time, const unsigned short int date ) const
	{
		Date ret;

		ret.second = time & 0x1f; // low 5 bytes
		ret.minute = time & 0x7e0; // middle 6 bytes
		ret.hour = time & 0xf800; // high 5 bytes

		ret.day = date & 0x1f; // low 5 bytes
		ret.month = date & 0x1e0;// middle 4 bytes
		ret.year = date & 0xfE00; // high 7 bytes

		return ret;
	}
	inline bool extractIfDeleted( const FatRawDirectoryEntry &rawDirEntry ) const
	{
		const char *ptr = reinterpret_cast<char*>( const_cast<FatRawDirectoryEntry*>( &rawDirEntry ) );

		return *ptr == DELETED_MAGIC;
	}
	inline size_t extractCluster( const FatRawDirectoryEntry &rawDirEntry ) const
	{
		return ( static_cast<size_t>( rawDirEntry.highCluster ) << 32 ) + rawDirEntry.lowCluster;
	}

public:
	DirectoryEntry() {}
	~DirectoryEntry() {}

	void assign( const std::vector<FatRawLongFileName> &longFileNames, const FatRawDirectoryEntry &rawDirEntry )
	{
		name = extractName( longFileNames, rawDirEntry );
		creationDate = extractDate( rawDirEntry.creationTime, rawDirEntry.creationDate );
		lastModificationDate = extractDate( rawDirEntry.lastModificationTime, rawDirEntry.lastModoficationDate );
		fileSize = rawDirEntry.fileSize;
		attributes = rawDirEntry.attributes;
		cluster = extractCluster( rawDirEntry );
		deleted = extractIfDeleted( rawDirEntry );
	}

	void print( std::wostream &ostream ) const
	{
		ostream << "Deleted: " << ( deleted ? "True" : "False" ) << "\n";
		ostream << "Name: " << name << "\n";
		ostream << "Size: " << fileSize << "\n";
		ostream << "Cluster: " << cluster << "\n";
		ostream << "Attributes: 0x" << std::hex << attributes << "\n\n";
	}
};

#endif