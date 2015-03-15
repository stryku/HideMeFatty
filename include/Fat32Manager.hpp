#ifndef _INCLUDE_FAT32MANAGER_
#define _INCLUDE_FAT32MANAGER_

#include <memory>
#include <vector>
#include <algorithm> 

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

#include <easyloggingpp_v9.80\easylogging++.h>

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
		ClusterWithFreeSpace( size_t clusterNo, size_t freeSpaceOffset );

		bool operator< ( const ClusterWithFreeSpace &c );
		bool operator> ( const ClusterWithFreeSpace &c );
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
	char* loadCluster( size_t clusterNo );

	inline size_t clusterSize() const;
	inline size_t getClusterFirstSectorNo( size_t clusterNo ) const;
	inline uintmax_t getClusterStartOffset( size_t clusterNo ) const;

	std::vector<FatRawLongFileName> extractLongFileNames( char *&ptrInCluster ) const;
	
	std::vector<size_t> getClusterChain( size_t firstCluster );
	std::vector<DirectoryEntry> getDirEntriesFromDirCluster( size_t dirCluster );
	std::vector<DirectoryEntry> getDirEntriesFromFolder( size_t firstCluster );
	std::vector<std::wstring> getPathFoldersNames( const std::wstring &path ) const;
	std::wstring getPathFileName( const std::wstring &path ) const;
	size_t getFreeSpaceAfterFile( const DirectoryEntry &fileDirEntry ) const;
	size_t getFileLastClusterNo( const DirectoryEntry &fileDirEntry ) const;
	ClusterInfo getFileLastClusterInfo( const DirectoryEntry &fileDirEntry );

	DirectoryEntry findNextDirEntry( size_t folderCluster, const DirectoryEntry &prevDirEntry = DirectoryEntry() );
	DirectoryEntry findDirEntryInFolder( std::wstring searchedDirEntryName, const size_t folderCluster );
	DirectoryEntry findFile( const std::wstring &path );
	
public:
	struct FreeSpaceChunk
	{
		uintmax_t offset;
		size_t size;

		FreeSpaceChunk( ) {}
		FreeSpaceChunk( uintmax_t offset, size_t size );

		bool operator< ( const FreeSpaceChunk &c ) const;
		friend std::ostream& operator<< ( std::ostream&, FreeSpaceChunk const& );
	};

	Fat32Manager();
	Fat32Manager( const std::wstring &partitionPath );
	~Fat32Manager() {}

	void setPartitionPath( const fs::path &partitionPath );

	bool isValidFat32();

	void close();

	bool isPathCorrect( const std::wstring &path );

	EFatType getFatType();
	size_t getFreeSpaceAfterFile( const std::wstring &path );
	size_t getFileLastClusterNo( const std::wstring &path );
	size_t getFileFreeSpaceOffset( const std::wstring &path );
	std::vector<FreeSpaceChunk> getSpacesAfterFiles( const std::vector<std::wstring> &files );

	char* mapSpaceAfterFiles( const std::vector<std::wstring> &files );
};

#endif