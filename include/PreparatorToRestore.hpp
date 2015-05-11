#ifndef _INLCUDE_PREPARATORTORESTORE_HPP_
#define _INLCUDE_PREPARATORTORESTORE_HPP_

#include <Preparator.hpp>

class PreparatorToRestore : public Preparator
{
private:
    const QString &pathToStore;


    bool checkPaths()
    {
        taskTree.newTask( "Checking paths on partition" );
        if( !Preparator::checkPaths( filesOnPartition ) )
        {
            taskTree.taskFailed();
            return false;
        }
        taskTree.taskSuccess();

        taskTree.newTask( "Checking path to store: " + pathToStore );
        if( !checkPath( pathToStore ) )
        {
            taskTree.taskFailed();
            return false;
        }
        taskTree.taskSuccess();

        taskTree.newTask( "Checking partition path: " + partitionPath );
        if( !checkPath( partitionPath )  )
        {
            taskTree.taskFailed();
            return false;
        }
        taskTree.taskSuccess();

        taskTree.newTask( "Checking partition device path: " + partitionDevPath );
        if( !checkPath( partitionDevPath ) )
        {
            taskTree.taskFailed();
            return false;
        }
        taskTree.taskSuccess();

        return true;
    }

public:
    PreparatorToRestore( QStringList &filesOnPartition,
                      const QString &partitionPath,
                      const QString &partitionDevPath,
                      const QString &pathToStore,
                      TaskTree &taskTree,
                      DistributedMemoryManager &dmm,
                      Fat32Manager &fatManager ) :
        Preparator( filesOnPartition,
                    partitionPath,
                    partitionDevPath,
                    taskTree,
                    dmm,
                    fatManager ),
        pathToStore( pathToStore )
    {}
    ~PreparatorToRestore() {}

    bool prepare()
    {
        taskTree.newTask( "Checking if paths are correct" );
        if( !checkPaths() )
        {
            taskTree.taskFailed();
            return false;
        }
        taskTree.taskSuccess();

        if( !commonPreparation() )
        {
            return false;
        }

        return true;
    }
};

#endif // _INLCUDE_PREPARATORTORESTORE_HPP_
