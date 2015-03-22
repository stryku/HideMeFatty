#include <DirectoryEntry.hpp>

DirectoryEntry::DirectoryEntry( ) :
	attributes( BAD_DIR_ENTRY )
{}

DirectoryEntry::DirectoryEntry( const std::vector<FatRawLongFileName> &longFileNames,
				const FatRawDirectoryEntry &rawDirEntry ) :
				attributes( BAD_DIR_ENTRY )
{
	assign( longFileNames, rawDirEntry );
}

inline std::string DirectoryEntry::getPartOfName( const FatRawLongFileName &longFileName ) const
{
	std::string namePart;

	for( size_t i = 0; i < 5; ++i )
	{
		if( longFileName.firtsChars[i] == '\0' )
			return namePart;

		namePart += longFileName.firtsChars[i];
	}

	for( size_t i = 0; i < 6; ++i )
	{
		if( longFileName.middleChars[i] == '\0' )
			return namePart;

		namePart += longFileName.middleChars[i];
	}

	for( size_t i = 0; i < 2; ++i )
	{
		if( longFileName.finalChars[i] == '\0' )
			return namePart;

		namePart += longFileName.finalChars[i];
	}

	return namePart;
}

inline std::string DirectoryEntry::extractExtension( const FatRawDirectoryEntry &rawDirEntry ) const
{
	std::string tempExtension;

	for( size_t i = 8; i < 11; ++i )
	{
		if( rawDirEntry.fileName[i] == END_OF_NAME )
			break;

		tempExtension += rawDirEntry.fileName[i];
	}

	return tempExtension;
}

inline std::string DirectoryEntry::extractShortName( const FatRawDirectoryEntry &rawDirEntry ) const
{
	std::string tempName, tempExtension;

	for( size_t i = 0; i < 8; ++i )
	{
		if( rawDirEntry.fileName[i] == END_OF_NAME )
			break;

		tempName += rawDirEntry.fileName[i];
	}

	tempExtension = extractExtension( rawDirEntry );

	if( tempExtension.length( ) != 0 )
	{
		tempName += '.';
		tempName += tempExtension;
	}

	return tempName;
}

inline std::string DirectoryEntry::extractName( const std::vector<FatRawLongFileName> &longFileNames,
												 const FatRawDirectoryEntry &rawDirEntry ) const
{
	std::string tempName;

	if( longFileNames.size( ) == 0 )
		tempName = extractShortName( rawDirEntry );
	else
	{
		for( const auto &i : longFileNames )
			tempName += getPartOfName( i );
	}

	return tempName;
}

inline Date DirectoryEntry::extractDate( const unsigned short int time, const unsigned short int date ) const
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

inline bool DirectoryEntry::extractIfDeleted( const FatRawDirectoryEntry &rawDirEntry ) const
{
	const char *ptr = reinterpret_cast<char*>( const_cast<FatRawDirectoryEntry*>( &rawDirEntry ) );

	return *ptr == DELETED_MAGIC;
}

inline size_t DirectoryEntry::extractCluster( const FatRawDirectoryEntry &rawDirEntry ) const
{
	return ( static_cast<uint32_t>( rawDirEntry.highCluster ) << 31 ) + rawDirEntry.lowCluster;
}

void DirectoryEntry::assign( const std::vector<FatRawLongFileName> &longFileNames, const FatRawDirectoryEntry &rawDirEntry )
{
	name = extractName( longFileNames, rawDirEntry );
	creationDate = extractDate( rawDirEntry.creationTime, rawDirEntry.creationDate );
	lastModificationDate = extractDate( rawDirEntry.lastModificationTime, rawDirEntry.lastModoficationDate );
	fileSize = rawDirEntry.fileSize;
	attributes = rawDirEntry.attributes;
	cluster = extractCluster( rawDirEntry );
	deleted = extractIfDeleted( rawDirEntry );
}

EDirEntryType DirectoryEntry::type( ) const
{
	return static_cast<EDirEntryType>( attributes );
}

std::string DirectoryEntry::getName( ) const
{
	return name;
}

uint64_t DirectoryEntry::getFileSize( ) const
{
	return fileSize;
}

size_t DirectoryEntry::getCluster( ) const
{
	return cluster;
}

void DirectoryEntry::setCluster( size_t cluster )
{
	this->cluster = cluster;
}

bool DirectoryEntry::operator==( const DirectoryEntry &de ) const
{
	return	de.name == name &&
		de.fileSize == fileSize &&
		de.attributes == attributes &&
		de.cluster == cluster &&
		de.creationDate == creationDate &&
		de.lastModificationDate == lastModificationDate;
}

std::ostream& operator<< ( std::ostream &out, const DirectoryEntry &de )
{
	out << std::dec;
	out << "\nname = " << de.name.c_str() <</*TODO*/ \
		"\nfile size = " << de.fileSize << \
		"\nattributes = 0x" << std::hex << static_cast<size_t>( de.attributes ) << \
		"\ncluster = " << std::dec << de.cluster << \
		"\ncreation date = " << de.creationDate << \
		"\nlast modification date = " << de.lastModificationDate << \
		"\ndeleted = " << std::boolalpha << de.deleted;

	return out;
}
