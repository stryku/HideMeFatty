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

#include <QStringList>

namespace fs = boost::filesystem;
//typedef std::vector<std::string> QStringList;

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

		ClusterWithFreeSpace() {}
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

	inline size_t getClusterFirstSectorNo( size_t clusterNo ) const;
	inline uint64_t getClusterStartOffset( size_t clusterNo ) const;

	std::vector<FatRawLongFileName> extractLongFileNames( char *&ptrInCluster ) const;

	std::vector<size_t> getClusterChain( size_t firstCluster );
	std::vector<DirectoryEntry> getDirEntriesFromDirCluster( size_t dirCluster );
	std::vector<DirectoryEntry> getDirEntriesFromFolder( size_t firstCluster );
    QStringList getPathFoldersNames( QString path ) const;
    QString getPathFileName( const QString &path ) const;
	size_t getFreeSpaceAfterFile( const DirectoryEntry &fileDirEntry ) const;
	size_t getFileLastClusterNo( const DirectoryEntry &fileDirEntry ) const;
	ClusterInfo getFileLastClusterInfo( const DirectoryEntry &fileDirEntry );
    size_t getFileFreeSpaceOffset( const DirectoryEntry &file );

	DirectoryEntry findNextDirEntry( size_t folderCluster, const DirectoryEntry &prevDirEntry = DirectoryEntry() );
    DirectoryEntry findDirEntryInFolder( QString searchedDirEntryName, const size_t folderCluster );
    DirectoryEntry findFile( const QString &path );
	
public:
	struct FreeSpaceChunk
	{
		uint64_t offset;
		size_t size;

		FreeSpaceChunk( ) {}
		FreeSpaceChunk( uint64_t offset, size_t size );

		bool operator< ( const FreeSpaceChunk &c ) const;
		friend std::ostream& operator<< ( std::ostream&, FreeSpaceChunk const& );
	};

	Fat32Manager();
    Fat32Manager( const QString &partitionPath );
	~Fat32Manager() {}

    void setPartitionPath( const QString &partitionPath );

	bool isValidFat32();
char *  mapChunks( std::vector<Fat32Manager::FreeSpaceChunk> chunks);
	void init();
	void close();
	bool good();
	void clear();

    bool isPathCorrect( const QString &path );

    size_t clusterSize() const;
	EFatType getFatType();
    size_t getFreeSpaceAfterFile( const QString &path );
    size_t getFileLastClusterNo( const QString &path );
    size_t getFileFreeSpaceOffset( const QString &path );
    std::vector<FreeSpaceChunk> getSpacesAfterFiles( const QStringList &files );

    char* mapSpaceAfterFiles( const QStringList &files );

	friend std::ostream& operator<< ( std::ostream&, Fat32Manager const& );

};

#endif
