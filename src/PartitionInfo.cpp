#include <PartitionInfo.hpp>

PartitionInfo::PartitionInfo( std::stringstream &in )
{
    std::string tmp;

    in >> tmp;
    devicePath = QString::fromStdString( tmp );
    in >> tmp;
    mediaPath = QString::fromStdString( tmp );
    in >> tmp;
    filesystem = QString::fromStdString( tmp );
    in >> tmp;
    attributes = QString::fromStdString( tmp );

    name = QFileInfo( QFile( mediaPath ) ).fileName();
}

void PartitionInfo::initClusterSize()
{
    if( devicePath.length() == 0 )
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

bool PartitionInfo::isValidFat32()
{
    if( devicePath.length() == 0 )
        return false;

    Fat32Manager fatManager;

    fatManager.setPartitionPath( devicePath );
    fatManager.init();

    if( !fatManager.good() )
        return false;

    return fatManager.isValidFat32();
}
