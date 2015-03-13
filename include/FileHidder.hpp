#ifndef _INCLUDE_FILEHIDDER_HPP_
#define _INCLUDE_FILEHIDDER_HPP_

#include <boost\filesystem\operations.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost\lexical_cast.hpp>
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

namespace fs = boost::filesystem;
using namespace pathOperations;

class FileHidder
{
public:
	struct HiddenChunkMetadata2
	{
		size_t nextClusterNo, offsetInNextCluster;

		HiddenChunkMetadata2( ) :
			nextClusterNo( magicEndOfChain )
		{}
		HiddenChunkMetadata2( const size_t _nextClusterNo,
							  const size_t _offsetInNextCluster ) :
							  nextClusterNo( _nextClusterNo ),
							  offsetInNextCluster( _offsetInNextCluster )
		{}

	};

	struct HiddenFileMetadata
	{
		static const size_t maxFileName = 256;

		uint64_t fileSize;
		wchar_t fileName[maxFileName];

		HiddenFileMetadata( ) {}

		HiddenFileMetadata( const std::wstring &fileName,
							const uint64_t fileSize ) :
							fileSize( fileSize )
		{
			std::copy( fileName.begin( ), fileName.end( ), this->fileName );
		}
	};
private:

	static const size_t magicEndOfChain = static_cast <size_t>( -1 );
	Fat32Manager fatManager;
	DistributedMemoryMapper dmm;

	bool isPathsCorrect( const std::vector<std::wstring> &paths )
	{
		for( const auto &path : paths )
		{
			if( !fatManager.isPathCorrect( path ) )
				return false;
		}

		return true;
	}

	uintmax_t getFilesSize( const std::vector<std::wstring> &filesPaths )
	{
		uintmax_t totalSize = std::accumulate( filesPaths.begin( ),
											   filesPaths.end( ),
											   0,
											   []( uintmax_t sum, std::wstring &path ) { return sum + fs::file_size( path ); } );

		return totalSize;
	}

	uintmax_t getSizeToHide( const std::vector<std::wstring> &filesToHide )
	{
		uintmax_t size;

		size = getFilesSize( filesToHide );
		size += filesToHide.size() * sizeof( HiddenFileMetadata );
		size += sizeof( uint64_t ); // for last 0 

		return size;
	}

	uintmax_t getFreeSpaceAfterFiles( const std::vector<std::wstring> &filesOnPartition )
	{
		uintmax_t totalSize = std::accumulate( filesOnPartition.begin( ),
											   filesOnPartition.end( ),
											   0,
											   [this]( uintmax_t sum, std::wstring &path ) 
												{ 
													return sum + fatManager.getFreeSpaceAfterFile( path );
												} );

		return totalSize;
	}

	std::string hashFile( const fs::path &path )
	{
		std::string result;
		CryptoPP::SHA1 hash;
		CryptoPP::FileSource( path.string( ).c_str( ), true,
							  new CryptoPP::HashFilter( hash, new CryptoPP::HexEncoder(
							  new CryptoPP::StringSink( result ), true ) ) );
		return result;
	}

	uint32_t getSeed( const std::vector<std::wstring> &filesOnPartition )
	{
		std::string stringSeed;
		CryptoPP::SHA1 sha1;
		std::stringstream ss;

		stringSeed = std::accumulate( filesOnPartition.begin( ),
									 filesOnPartition.end( ),
									 std::string(""),
									 [this]( std::string sum, std::wstring &path ) { return sum + hashFile( path ); } );

		CryptoPP::StringSource( stringSeed, 
								true, 
								new CryptoPP::HashFilter( sha1, new CryptoPP::HexEncoder( new CryptoPP::StringSink( stringSeed ) ) ) );

		stringSeed = stringSeed.substr( 0, 8 );

		return boost::lexical_cast<uint32_t>( stringSeed );
	}

	bool mapFreeSpace( const std::vector<std::wstring> &filesOnPartition )
	{
		std::vector<Fat32Manager::FreeSpaceChunk> chunks;
		uintmax_t startOffset;
		char *mappedPtr;
		
		mappedPtr = fatManager.mapSpaceAfterFiles( filesOnPartition );

		if( mappedPtr == nullptr )
			return false;

		chunks = fatManager.getSpacesAfterFiles( filesOnPartition );

		startOffset = std::min( chunks.begin(), chunks.end() )->offset;

		for( const auto &chunk : chunks )
			dmm.addMemoryChunk( mappedPtr + ( chunk.offset - startOffset ), chunk.size );

		return true;
	}

	void hideMetadata( const HiddenFileMetadata &metadata, boost::random::mt19937 &rng, const uintmax_t freeSpaceSize )
	{
		const char *data = reinterpret_cast<const char*>( &metadata );;

		for( size_t i = 0; i < sizeof( HiddenFileMetadata ); ++i )
			dmm[rng() % freeSpaceSize] = data[i];
	}

	bool hideFileContents( const std::wstring &filePath, boost::random::mt19937 &rng, const uintmax_t freeSpaceSize )
	{
		uintmax_t fileSize;
		char ch;
		std::ifstream file( filePath, std::ios::binary );

		if( !file.is_open( ) )
			return false;

		fileSize = fs::file_size( filePath );

		for( size_t i = 0; i < fileSize; ++i )
		{
			ch = file.get();
			dmm[rng() % freeSpaceSize] = ch;
		}

		return true;
	}

	bool hideFile( const std::wstring &filePath, boost::random::mt19937 &rng, const uintmax_t freeSpaceSize )
	{
		HiddenFileMetadata fileMetadata( getPathFileName( filePath ),
										 fs::file_size( filePath ) );

		hideMetadata( fileMetadata, rng, freeSpaceSize );
		
		return hideFileContents( filePath, rng, freeSpaceSize );
	}

public:
	FileHidder() {}
	~FileHidder() {}

	bool hideFile2( const std::string &filePath, const std::string &partitionPath )
	{
		const size_t metadataSize = sizeof( HiddenChunkMetadata );

		std::vector<ClusterInfo> clustersWithFreeBytes;
		uintmax_t fileSize,
			copiedBytes = 0;
		MappedFileManager mappedFileMngr;
		char *mappedPtr;
		HiddenChunkMetadata hiddenChunkMetadata;

		fatManager.setPartitionPath( partitionPath );

		fileSize = fs::file_size( filePath );

		if( fileSize == static_cast<uintmax_t>( -1 ) )
			return false;

		clustersWithFreeBytes = fatManager.getClustersWithFreeBytes( fileSize, metadataSize );

		if( clustersWithFreeBytes.size() == 0 )
			return false;

		mappedFileMngr.setFilePath( filePath );
		mappedPtr = mappedFileMngr.map( );

		fatManager.writeToCluster( clustersWithFreeBytes[0].clusterNo,
								   clustersWithFreeBytes[0].freeBytesOffset,
								   sizeof( uintmax_t ),
								   reinterpret_cast<char*>( &fileSize ) );

		clustersWithFreeBytes[0].freeBytesOffset += sizeof( uintmax_t );
		clustersWithFreeBytes[0].freeBytes -= sizeof( uintmax_t );

		for( auto it = clustersWithFreeBytes.begin(); it != clustersWithFreeBytes.end(); ++it )
		{
			size_t bytesToCopy = it->freeBytes - metadataSize;
			auto nextCluster = it + 1;

			if( nextCluster == clustersWithFreeBytes.end() )
				hiddenChunkMetadata.nextClusterNo = magicEndOfChain;
			else
			{
				hiddenChunkMetadata.nextClusterNo = nextCluster->clusterNo;
				hiddenChunkMetadata.offsetInNextCluster = nextCluster->freeBytesOffset;
			}

			fatManager.writeToCluster( it->clusterNo,
									   it->freeBytesOffset,
									   metadataSize,
									   reinterpret_cast<char*>  ( &hiddenChunkMetadata ) );

			fatManager.writeToCluster( it->clusterNo,
									   it->freeBytesOffset + metadataSize,
									   bytesToCopy,
									   mappedPtr + copiedBytes );

			copiedBytes += bytesToCopy;
		}

		return true;
	}

	

	bool hideFiles( const std::vector<std::wstring> &filesOnPartition,
					const std::vector<std::wstring> &filesToHide,
					const std::wstring &partitionPath )
	{
		uintmax_t freeSpaceSize;
		uint32_t seed;
		boost::random::mt19937 rng;

		if( !isPathsCorrect( filesOnPartition ) )
			return false;

		freeSpaceSize = getFreeSpaceAfterFiles( filesOnPartition );

		if( getSizeToHide( filesToHide ) > freeSpaceSize )
			return false;

		if( !mapFreeSpace( filesOnPartition ) )
			return false;

		seed = getSeed( filesOnPartition );

		rng.seed( seed );

		for( const auto &file : filesToHide )
		{
			if( !hideFile( file, rng, freeSpaceSize ) )
				return false;
		}

		return true;
	}
};

#endif