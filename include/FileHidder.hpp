#ifndef _INCLUDE_FILEHIDDER_HPP_
#define _INCLUDE_FILEHIDDER_HPP_

#include <boost\filesystem\operations.hpp>
#include <vector>

#include <Fat32Manager.hpp>

namespace fs = boost::filesystem;

class FileHidder
{
public:
	struct HiddenChunkMetadata
	{
		size_t nextClusterNo, offsetInNextCluster;

		HiddenChunkMetadata() :
			nextClusterNo( magicEndOfChain )
		{}
		HiddenChunkMetadata( const size_t _nextClusterNo,
							 const size_t _offsetInNextCluster ) :
							 nextClusterNo( _nextClusterNo ),
							 offsetInNextCluster( _offsetInNextCluster )
		{}
						
	};

private:
	static const size_t magicEndOfChain = static_cast <size_t>( -1 );
	Fat32Manager fatManager;

public:
	FileHidder() {}
	~FileHidder() {}

	bool hideFile( const std::string &filePath, const std::string &partitionPath )
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
};

#endif