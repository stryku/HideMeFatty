#ifndef _INCLUDE_PARTITIONINFO_HPP_
#define _INCLUDE_PARTITIONINFO_HPP_

#include <Fat32Manager.hpp>
#include <QDataStream>

struct PartitionInfo
{
    QString devicePath,
            mediaPath,
            name,
            filesystem,
            attributes;

    size_t clusterSize;

    PartitionInfo() {}
    PartitionInfo( std::stringstream &in );
    ~PartitionInfo() {}

    void initClusterSize();
};

#endif // _INCLUDE_PARTITIONINFO_HPP_
