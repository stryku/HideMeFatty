#ifndef _INCLUDE_PARTITIONFINDER_HPP_
#define _INCLUDE_PARTITIONFINDER_HPP_

#include <fstream>
#include <string>
#include <sstream>
#include <PartitionInfo.hpp>

class PartitionFinder
{
private:

public:
    PartitionFinder() {}
    ~PartitionFinder() {}

    std::vector<PartitionInfo> getMountedPartitions()
    {
        std::ifstream mtab( "/etc/mtab" );
        std::string line;
        std::vector<PartitionInfo> ret;

        while( getline(mtab, line) )
        {
            std::stringstream lineStream( line );

            ret.push_back( PartitionInfo( lineStream ) );
        }

        return ret;
    }
};

#endif // _INCLUDE_PARTITIONFINDER_HPP_
