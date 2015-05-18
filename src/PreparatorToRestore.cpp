#include <PreparatorToRestore.hpp>

bool PreparatorToRestore::checkPaths()
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

bool PreparatorToRestore::prepare()
{
    taskTree.newTask( "Checking if paths are correct" );
    if( !checkPaths() )
    {
        taskTree.taskFailed();
        return false;
    }
    taskTree.taskSuccess();

    if( !commonPreparation() )
        return false;

    return true;
}
