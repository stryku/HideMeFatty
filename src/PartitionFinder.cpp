#include <PartitionFinder.hpp>

std::vector<PartitionInfo> PartitionFinder::getMountedPartitions()
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
