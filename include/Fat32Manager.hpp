#ifndef _INCLUDE_FAT32MANAGER_
#define _INCLUDE_FAT32MANAGER_

#include <memory>
#include <vector>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

#include <FatStructs.h>
#include <MappedFileManager.hpp>
#include <DirectoryEntry.hpp>

namespace fs = boost::filesystem;

class Fat32Manager
{
private:
	static const size_t bootSectorSize = 512;
	static const unsigned char DELETED_MAGIC = 0xE5;
	static const unsigned char LFN_ATTRIBUTE = 0x0F;
	const size_t lastClusterMagic = 0x0FFFFFF8;

	struct ClusterWithFreeSpace
	{
		size_t clusterNo,
		freeSpaceOffset;

		ClusterWithFreeSpace( ) {}
		ClusterWithFreeSpace( size_t, size_t );

		bool operator< ( const ClusterWithFreeSpace & );
		bool operator> ( const ClusterWithFreeSpace & );
	};

	FatBS bootSector;
	Fat32ExtBS fat32ExtBS;
	FatInfo fatInfo;
	bool fatInfoLoaded, bootSectorLoaded, fatTableLoaded, initOk;
	MappedFileManager mappedFileMngr;
	std::vector<uint32_t> fatTable;
	
private:

	bool init();

	void clear();

	void loadFat32ExtBS();
	bool loadBootSector();
	void loadFatInfo();
	bool loadFatTable();
	char* loadCluster( size_t );

	inline size_t clusterSize() const;
	inline size_t getClusterFirstSectorNo( size_t ) const;
	inline uintmax_t getClusterStartOffset( size_t ) const;

	std::vector<FatRawLongFileName> extractLongFileNames( char *& ) const;
	
	std::vector<size_t> getClusterChain( size_t );
	std::vector<DirectoryEntry> getDirEntriesFromDirCluster( size_t );
	std::vector<DirectoryEntry> getDirEntriesFromFolder( size_t );
	std::vector<std::wstring> getPathFoldersNames( const std::wstring & ) const;
	std::wstring getPathFileName( const std::wstring & ) const;
	size_t getFreeSpaceAfterFile( const DirectoryEntry & ) const;
	size_t getFileLastClusterNo( const DirectoryEntry & ) const;
	ClusterInfo getFileLastClusterInfo( const DirectoryEntry & );

	DirectoryEntry findNextDirEntry( size_t , const DirectoryEntry & = DirectoryEntry() );
	DirectoryEntry findNextFile( size_t folderCluster, const DirectoryEntry & = DirectoryEntry() );
	DirectoryEntry findDirEntryInFolder( std::wstring , const size_t  );
	DirectoryEntry findFile( const std::wstring & );
	
public:
	struct FreeSpaceChunk
	{
		uintmax_t offset;
		size_t size;

		FreeSpaceChunk( ) {}
		FreeSpaceChunk( uintmax_t , size_t  );

		bool operator< ( const FreeSpaceChunk & ) const;
	};

	Fat32Manager();
	Fat32Manager( const std::string & );
	~Fat32Manager() {}

	void setPartitionPath( const fs::path & );

	bool isValidFat32();

	void close();

	bool isPathCorrect( const std::wstring & );

	EFatType getFatType();
	size_t getFreeSpaceAfterFile( const std::wstring & );
	size_t getFileLastClusterNo( const std::wstring & );
	size_t getFileFreeSpaceOffset( const std::wstring & );
	std::vector<FreeSpaceChunk> getSpacesAfterFiles( const std::vector<std::wstring> & );

	char* mapSpaceAfterFiles( const std::vector<std::wstring> & );
};

#endif