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

#include <cryptopp562\sha.h>
#include <cryptopp562\hex.h>
#include <cryptopp562\files.h> 

#include <easyloggingpp_v9.80\easylogging++.h>

#include <vector>
#include <string>
#include <numeric>
#include <fstream>

#include <Fat32Manager.hpp>
#include <DistributedMemoryMapper.hpp>
#include <pathOperations.hpp>

#include <iostream>

namespace fs = boost::filesystem;
using namespace pathOperations;

class FileHidder
{
public:
	struct HiddenFileMetadata
	{
		static const size_t maxFileName = 256;
		static const size_t fileNameBytesSize = maxFileName*sizeof( wchar_t );

		uint64_t fileSize;
		wchar_t fileName[maxFileName];

		HiddenFileMetadata();
		HiddenFileMetadata( const std::wstring &fileName,
							const uint64_t fileSize );
	};

private:

	Fat32Manager fatManager;
	DistributedMemoryMapper dmm;

	bool isPathsCorrect( const std::vector<std::wstring> &paths, const std::wstring &partitionPath );

	uintmax_t getFilesSize( const std::vector<std::wstring> &filesPaths );
	uintmax_t getSizeToHide( const std::vector<std::wstring> &filesToHide );
	uintmax_t getFreeSpaceAfterFiles( const std::vector<std::wstring> &filesOnPartition );
	uint32_t getSeed( const std::vector<std::wstring> &filesOnPartition );

	std::string hashFile( const fs::path &path );

	bool mapFreeSpace( const std::vector<std::wstring> &filesOnPartition );

	std::vector<std::wstring> preparePathsOnPartition( const std::vector<std::wstring> &filesOnPartition,
													   const std::wstring &partitionPath ) const;

	void hideFileSize( const uintmax_t &fileSize );
	void hideFileName( const wchar_t *fileName );
	void hideMetadata( const HiddenFileMetadata &metadata, boost::random::mt19937 &rng, const uintmax_t freeSpaceSize );
	bool hideFileContents( const std::wstring &filePath, boost::random::mt19937 &rng, const uintmax_t freeSpaceSize );
	bool hideFile( const std::wstring &filePath, boost::random::mt19937 &rng, const uintmax_t freeSpaceSize );

	uintmax_t restoreFileSize();
	void restoreFileName( HiddenFileMetadata &metadata );
	HiddenFileMetadata restoreMetadata( boost::random::mt19937 &rng, const uintmax_t freeSpaceSize );
	void restoreFile( std::ofstream &fileStream,
					  boost::random::mt19937 &rng,
					  const uintmax_t freeSpaceSize,
					  const HiddenFileMetadata &metadata );
	bool restoreMyFile( const std::wstring &pathToStore,
						boost::random::mt19937 &rng,
						const uintmax_t freeSpaceSize );

	bool prepareFatManager( const std::wstring &partitionPath );
	bool checkPaths( const std::vector<std::wstring> &paths );
	bool checkPaths( const std::vector<std::wstring> &filesOnPartition,
					 const std::wstring &partitionPath,
					 const std::vector<std::wstring> &filesToHide,
					 const std::wstring &partitionDevPath );
	bool checkPaths( const std::vector<std::wstring> &filesOnPartition,
					 const std::wstring &partitionPath,
					 const std::wstring &partitionDevPath,
					 const std::wstring &pathToStore );

public:
	FileHidder() {}
	~FileHidder() {}

	bool hideFiles( const std::vector<std::wstring> &filesOnPartition,
					const std::wstring &partitionPath,
					const std::vector<std::wstring> &filesToHide,
					const std::wstring &partitionDevPath );

	bool restoreMyFiles( const std::vector<std::wstring> &filesOnPartition,
						 const std::wstring &partitionPath,
						 const std::wstring &partitionDevPath,
						 const std::wstring &pathToStore );
};

#endif