#ifndef _INCLUDE_FILEHIDER_HPP_
#define _INCLUDE_FILEHIDER_HPP_

#ifdef _MSC_VER
// Crypto++ Library
#ifdef _DEBUG
#  pragma comment ( lib, "cryptlib" )
#else
#  pragma comment ( lib, "cryptlib" )
#endif
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/nowide/fstream.hpp>

#include <crypto++/sha.h>
#include <crypto++/hex.h>
#include <crypto++/files.h>
#include <crypto++/base64.h>

#include <easylogging++.h>

#include <vector>
#include <numeric>
#include <algorithm>
#include <string>
#include <map>

#include <Fat32Manager.hpp>
#include <DistributedMemoryMapper.hpp>
#include <pathOperations.hpp>

#include <iostream>

namespace fs = boost::filesystem;
using namespace boost::nowide;
using namespace pathOperations;

typedef std::vector<std::string> StringVector;

class FileHider
{
public:
	struct HiddenFileMetadata
	{
		static const size_t maxFileName = 256;

		uint64_t fileSize;
		char fileName[maxFileName];

		HiddenFileMetadata();
		HiddenFileMetadata( const std::string &fileName,
							const uint64_t fileSize );
	};

private:

	Fat32Manager fatManager;
	DistributedMemoryMapper dmm;

	bool isPathsCorrect( const StringVector &paths, const std::string &partitionPath );

	uint64_t getFilesSize( const StringVector &filesPaths );
	uint64_t getSizeToHide( const StringVector &filesToHide );
	uint64_t getFreeSpaceAfterFiles( const StringVector &filesOnPartition );
	uint32_t getSeed( const StringVector &filesOnPartition );

	std::string hashFile( const std::string &path );

	bool mapFreeSpace( const StringVector &filesOnPartition );

	StringVector preparePathsOnPartition( const StringVector &filesOnPartition,
													  const std::string &partitionPath ) const;
	std::string preparePathToStore( const std::string &pathToStore,
									const FileHider::HiddenFileMetadata &fileMetadata,
									std::map<std::string, size_t> &restoredFiles ) const;

	void hideFileSize( const uint64_t &fileSize );
	void hideFileName( const char *fileName );
	void hideMetadata( const HiddenFileMetadata &metadata, boost::random::mt19937 &rng, const uint64_t freeSpaceSize );
	bool hideFileContents( const std::string &filePath, boost::random::mt19937 &rng, const uint64_t freeSpaceSize );
	bool hideFile( const std::string &filePath, boost::random::mt19937 &rng, const uint64_t freeSpaceSize );

	uint64_t restoreFileSize();
	void restoreFileName( HiddenFileMetadata &metadata );
	HiddenFileMetadata restoreMetadata( boost::random::mt19937 &rng, const uint64_t freeSpaceSize );
	void restoreFile( std::ofstream &fileStream,
					  boost::random::mt19937 &rng,
					  const uint64_t freeSpaceSize,
					  const HiddenFileMetadata &metadata );
	bool restoreMyFile( std::string pathToStore,
						boost::random::mt19937 &rng,
						const uint64_t freeSpaceSize,
					  std::map<std::string, size_t> &restoredFiles );

	bool prepareFatManager( const std::string &partitionPath );
	bool checkPaths( const StringVector &paths );
	bool checkPaths( const StringVector &filesOnPartition,
					 const std::string &partitionPath,
					 const StringVector &filesToHide,
					 const std::string &partitionDevPath );
	bool checkPaths( const StringVector &filesOnPartition,
					 const std::string &partitionPath,
					 const std::string &partitionDevPath,
					 const std::string &pathToStore );

public:
	FileHider() {}
	~FileHider() {}

	bool hideFiles( StringVector &filesOnPartition,
					const std::string &partitionPath,
					const StringVector &filesToHide,
					const std::string &partitionDevPath );

	bool restoreMyFiles( StringVector &filesOnPartition,
						 const std::string &partitionPath,
						 const std::string &partitionDevPath,
						 const std::string &pathToStore );
};

#endif
