#ifndef _INCLUDE_FILEHIDDER_HPP_
#define _INCLUDE_FILEHIDDER_HPP_

#include <boost\filesystem\operations.hpp>
#include <vector>

#include <Fat32Manager.hpp>

namespace fs = boost::filesystem;

class FileHidder
{
private:
	static const size_t magicEndOfChain = static_cast <size_t>( -1 );
	Fat32Manager fatManager;

public:
	FileHidder() {}
	~FileHidder() {}

	bool hideFile( const std::string &filePath, const std::string &partitionPath )
	{
		const int badFileMagic = -1;

		std::vector<ClusterInfo> clustersWithFreeBytes;
		uintmax_t fileSize,
			copiedBytes = 0;
		MappedFileManager mappedFileMngr;
		char *mappedPtr;

		fatManager.setPartitionPath( partitionPath );

		fileSize = fs::file_size( filePath );

		if( fileSize == static_cast<uintmax_t>( -1 ) )
			return false;

		clustersWithFreeBytes = fatManager.getClustersWithFreeBytes( fileSize );

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
			size_t bytesToCopy = it->freeBytes - sizeof( size_t ),
				nextClusterNo;
			auto nextCluster = it + 1;

			fatManager.writeToCluster( it->clusterNo,
									   it->freeBytesOffset,
									   bytesToCopy,
									   mappedPtr + copiedBytes );

			if( nextCluster == clustersWithFreeBytes.end() )
				nextClusterNo = magicEndOfChain;
			else
				nextClusterNo = nextCluster->clusterNo;

			fatManager.writeToEndOfCluster( it->clusterNo,
											sizeof( size_t ),
											reinterpret_cast<char*>( &nextClusterNo ) );

			copiedBytes += bytesToCopy;
		}

		return true;
	}
};

#endif