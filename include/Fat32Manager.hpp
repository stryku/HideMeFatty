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

	bool _init();


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
	std::vector<std::string> getPathFoldersNames( const std::string &path ) const;
	std::string getPathFileName( const std::string &path ) const;
	size_t getFreeSpaceAfterFile( const DirectoryEntry &fileDirEntry ) const;
	size_t getFileLastClusterNo( const DirectoryEntry &fileDirEntry ) const;
	ClusterInfo getFileLastClusterInfo( const DirectoryEntry &fileDirEntry );

	DirectoryEntry findNextDirEntry( size_t folderCluster, const DirectoryEntry &prevDirEntry = DirectoryEntry() );
	DirectoryEntry findDirEntryInFolder( std::string searchedDirEntryName, const size_t folderCluster );
	DirectoryEntry findFile( const std::string &path );
	
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
	Fat32Manager( const std::string &partitionPath );
	~Fat32Manager() {}

	void setPartitionPath( const fs::path &partitionPath );

	bool isValidFat32();

	void init();
	void close();
	bool good();
	void clear( );

	bool isPathCorrect( const std::string &path );

	EFatType getFatType();
	size_t getFreeSpaceAfterFile( const std::string &path );
	size_t getFileLastClusterNo( const std::string &path );
	size_t getFileFreeSpaceOffset( const std::string &path );
	std::vector<FreeSpaceChunk> getSpacesAfterFiles( const std::vector<std::string> &files );

	char* mapSpaceAfterFiles( const std::vector<std::string> &files );

	friend std::ostream& operator<< ( std::ostream&, Fat32Manager const& );
};

#endif