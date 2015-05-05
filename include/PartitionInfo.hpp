#ifndef _INCLUDE_PARTITIONINFO_HPP_
#define _INCLUDE_PARTITIONINFO_HPP_

//todo split to cpp file

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

    PartitionInfo( std::stringstream &in )
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

    ~PartitionInfo() {}

    void initClusterSize()
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
};

#endif // _INCLUDE_PARTITIONINFO_HPP_
