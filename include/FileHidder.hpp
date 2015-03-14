#ifndef _INCLUDE_FILEHIDDER_HPP_
#define _INCLUDE_FILEHIDDER_HPP_

// Crypto++ Library
#ifdef _DEBUG
#  pragma comment ( lib, "cryptlib" )
#else
#  pragma comment ( lib, "cryptlib" )
#endif

#include <boost\filesystem\operations.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost\lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <cryptopp562\sha.h>
#include <cryptopp562\hex.h>
#include <cryptopp562\files.h>
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
		HiddenFileMetadata( const std::wstring &,
							const uint64_t );
	};

private:

	Fat32Manager fatManager;
	DistributedMemoryMapper dmm;

	bool isPathsCorrect( const std::vector<std::wstring> &, const std::wstring & );

	uintmax_t getFilesSize( const std::vector<std::wstring> & );
	uintmax_t getSizeToHide( const std::vector<std::wstring> & );
	uintmax_t getFreeSpaceAfterFiles( const std::vector<std::wstring> & );
	uint32_t getSeed( const std::vector<std::wstring> & );

	std::string hashFile( const fs::path & );

	bool mapFreeSpace( const std::vector<std::wstring> & );

	std::vector<std::wstring> preparePathsOnPartition( const std::vector<std::wstring> &,
													   const std::wstring & ) const;

	void hideFileSize( const uintmax_t & );
	void hideFileName( const wchar_t * );
	void hideMetadata( const HiddenFileMetadata &, boost::random::mt19937 &, const uintmax_t  );
	bool hideFileContents( const std::wstring &, boost::random::mt19937 &, const uintmax_t  );
	bool hideFile( const std::wstring &, boost::random::mt19937 &, const uintmax_t  );

	uintmax_t restoreFileSize();
	void restoreFileName( HiddenFileMetadata & );
	HiddenFileMetadata restoreMetadata( boost::random::mt19937 &, const size_t  );
	void restoreFile( std::ofstream &,
					  boost::random::mt19937 &,
					  const size_t ,
					  const HiddenFileMetadata & );
	bool restoreMyFile( const std::wstring &,
						boost::random::mt19937 &,
						const size_t  );

	

public:
	FileHidder() {}
	~FileHidder() {}

	bool hideFiles( const std::vector<std::wstring> &,
					const std::wstring &,
					const std::vector<std::wstring> &,
					const std::wstring & );

	bool restoreMyFiles( const std::vector<std::wstring> &,
						 const std::wstring &,
						 const std::wstring &,
						 const std::wstring & );
};

#endif