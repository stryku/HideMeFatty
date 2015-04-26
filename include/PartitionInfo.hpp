#ifndef _INCLUDE_PARTITIONINFO_HPP_
#define _INCLUDE_PARTITIONINFO_HPP_

#include <Fat32Manager.hpp>

struct PartitionInfo
{
    std::string devicePath,
                mediaPath,
                name,
                filesystem,
                attributes;

    size_t clusterSize;

    PartitionInfo() {}

    PartitionInfo( std::stringstream &in )
    {
        in  >> devicePath \
            >> mediaPath \
            >> filesystem \
            >> attributes;

        name = pathOperations::getPathFileName( mediaPath );
    }

    ~PartitionInfo() {}

    void initClusterSize()
    {
        if( devicePath.empty() )
            clusterSize = 0;
        else
        {
            Fat32Manager fatManager;

            fatManager.setPartitionPath( devicePath );
            fatManager.init();

            if( fatManager.good() )
                clusterSize = fatManager.clusterSize();
            else
                clusterSize = 0;
        }
    }
};

#endif // _INCLUDE_PARTITIONINFO_HPP_
