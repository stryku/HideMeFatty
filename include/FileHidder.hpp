#ifndef _INCLUDE_FILEHIDDER_HPP_
#define _INCLUDE_FILEHIDDER_HPP_

#include <boost\filesystem\operations.hpp>
#include <vector>
#include <string>

#include <Fat32Manager.hpp>

namespace fs = boost::filesystem;

class FileHidder
{
private:
	static const size_t magicEndOfChain = static_cast <size_t>( -1 );
	static const size_t maxFileName = 256;
	Fat32Manager fatManager;

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
		uintmax_t size = 0;

		for( const auto &path : filesPaths )
			size += fs::file_size( path );

		return size;
	}

	uintmax_t getSizeToHide( const std::vector<std::wstring> &filesToHide )
	{
		uintmax_t size;

		size = getFilesSize( filesToHide );
		size += filesToHide.size() * sizeof( HiddenFileMetadata );
		size += sizeof( uint64_t ); // for last 0 

		return size;
	}

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
		wchar_t fileName[maxFileName];
		uint64_t fileSize;

		HiddenFileMetadata( ) {}

		HiddenFileMetadata( const std::wstring &fileName,
							 const uint64_t fileSize ) :
							 fileSize( fileSize )
		{
			std::copy( fileName.begin(), fileName.end(), this->fileName );
		}
	};


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
		uintmax_t sizeToHide;

		if( !isPathsCorrect( filesOnPartition ) )
			return false;


	}
};

#endif