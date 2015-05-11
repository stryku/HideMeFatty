#ifndef _INCLUDE_PREPARATOR_HPP_
#define _INCLUDE_PREPARATOR_HPP_

#include <QStringList>

#include <TaskTree.hpp>
#include <DistributedMemoryManager.hpp>
#include <Fat32Manager.hpp>

class Preparator
{
protected:
    QStringList &filesOnPartition;
    QString &partitionPath;
    QString &partitionDevPath;
    TaskTree &taskTree;
    DistributedMemoryManager &dmm;
    Fat32Manager &fatManager;

    bool fatManagerPrepared;

    bool commonPreparation()
    {
        uint32_t seed;
        QStringList preparedPaths;

        taskTree.newTask( "Preparing paths on parition" );
        std::sort( filesOnPartition.begin( ),
                   filesOnPartition.end( ) );

        preparedPaths = preparePathsOnPartition( filesOnPartition, partitionPath );
        taskTree.taskSuccess();

        if( !fatManagerPrepared )
        {
            taskTree.newTask( "Preparing FAT manager" );
            if( prepareFatManager( partitionDevPath ) == false )
            {
                taskTree.taskFailed();
                return false;
            }
            taskTree.taskSuccess();
        }

        taskTree.newTask( "Calculating seed to hash" );
        seed = getSeed( filesOnPartition );
        taskTree.taskSuccess();

        taskTree.newTask( "Mapping space after files on partition" );
        if( !mapFreeSpace( preparedPaths ) )
        {
            taskTree.taskFailed();
            return false;
        }
        taskTree.taskSuccess();

        taskTree.newTask( "Creating shuffled array of bytes after files on partition" );
        dmm.createShuffledArray( seed );
        taskTree.taskSuccess();

        return true;
    }

public:
    Preparator( const QStringList &filesOnPartition,
                const QString &partitionPath,
                const QString &partitionDevPath,
                TaskTree &taskTree,
                DistributedMemoryManager &dmm,
                Fat32Manager &fatManager ) :
        filesOnPartition( filesOnPartition ),
        partitionPath( partitionPath ),
        partitionDevPath( partitionDevPath ),
        taskTree( taskTree ),
        dmm( dmm ),
        fatManager( fatManager ),
        fatManagerPrepared( false )
    {}
    ~Preparator() {}

    virtual void prepare() = 0;
};

#endif // _INCLUDE_PREPARATOR_HPP_
