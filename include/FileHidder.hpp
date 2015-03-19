#ifndef _INCLUDE_FILEHIDDER_HPP_
#define _INCLUDE_FILEHIDDER_HPP_

// Crypto++ Library
#ifdef _DEBUG
#  pragma comment ( lib, "cryptlib" )
#else
#  pragma comment ( lib, "cryptlib" )
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/nowide/fstream.hpp>

#include <crypto++/sha.h>
#include <crypto++/hex.h>
#include <crypto++/files.h>

#include <easylogging++.h>

#include <vector>
#include <numeric>
#include <algorithm>
#include <string>

#include <Fat32Manager.hpp>
#include <DistributedMemoryMapper.hpp>
#include <pathOperations.hpp>

#include <iostream>

namespace fs = boost::filesystem;
using namespace boost::nowide;
using namespace pathOperations;

class FileHidder
{
public:
	struct HiddenFileMetadata
	{
		static const size_t maxFileName = 256;

		uint64_t fileSize;
		char fileName[maxFileName];

		HiddenFileMetadata();
		HiddenFileMetadata( const std::string &fileName,
							const uintmax_t fileSize );
	};

private:

	Fat32Manager fatManager;
	DistributedMemoryMapper dmm;

	bool isPathsCorrect( const std::vector<std::string> &paths, const std::string &partitionPath );

	uintmax_t getFilesSize( const std::vector<std::string> &filesPaths );
	uintmax_t getSizeToHide( const std::vector<std::string> &filesToHide );
	uintmax_t getFreeSpaceAfterFiles( const std::vector<std::string> &filesOnPartition );
	uint32_t getSeed( std::vector<std::string> &filesOnPartition );

	std::string hashFile( const fs::path &path );

	bool mapFreeSpace( const std::vector<std::string> &filesOnPartition );

	std::vector<std::string> preparePathsOnPartition( const std::vector<std::string> &filesOnPartition,
													  const std::string &partitionPath ) const;

	void hideFileSize( const uintmax_t &fileSize );
	void hideFileName( const char *fileName );
	void hideMetadata( const HiddenFileMetadata &metadata, boost::random::mt19937 &rng, const uintmax_t freeSpaceSize );
	bool hideFileContents( const std::string &filePath, boost::random::mt19937 &rng, const uintmax_t freeSpaceSize );
	bool hideFile( const std::string &filePath, boost::random::mt19937 &rng, const uintmax_t freeSpaceSize );

	uintmax_t restoreFileSize();
	void restoreFileName( HiddenFileMetadata &metadata );
	HiddenFileMetadata restoreMetadata( boost::random::mt19937 &rng, const uintmax_t freeSpaceSize );
	void restoreFile( std::ofstream &fileStream,
					  boost::random::mt19937 &rng,
					  const uintmax_t freeSpaceSize,
					  const HiddenFileMetadata &metadata );
	bool restoreMyFile( std::string pathToStore,
						boost::random::mt19937 &rng,
						const uintmax_t freeSpaceSize );

	bool prepareFatManager( const std::string &partitionPath );
	bool checkPaths( const std::vector<std::string> &paths );
	bool checkPaths( const std::vector<std::string> &filesOnPartition,
					 const std::string &partitionPath,
					 const std::vector<std::string> &filesToHide,
					 const std::string &partitionDevPath );
	bool checkPaths( const std::vector<std::string> &filesOnPartition,
					 const std::string &partitionPath,
					 const std::string &partitionDevPath,
					 const std::string &pathToStore );

public:
	FileHidder() {}
	~FileHidder() {}

	bool hideFiles( std::vector<std::string> &filesOnPartition,
					const std::string &partitionPath,
					const std::vector<std::string> &filesToHide,
					const std::string &partitionDevPath );

	bool restoreMyFiles( std::vector<std::string> &filesOnPartition,
						 const std::string &partitionPath,
						 const std::string &partitionDevPath,
						 const std::string &pathToStore );
};

#endif
