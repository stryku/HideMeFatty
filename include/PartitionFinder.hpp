#ifndef _INCLUDE_PARTITIONFINDER_HPP_
#define _INCLUDE_PARTITIONFINDER_HPP_

#include <fstream>
#include <string>
#include <sstream>
#include <PartitionInfo.hpp>

class PartitionFinder
{
public:
    PartitionFinder() {}
    ~PartitionFinder() {}

    std::vector<PartitionInfo> getMountedPartitions();
};

#endif // _INCLUDE_PARTITIONFINDER_HPP_
