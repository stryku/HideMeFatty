#include <PreparatorToHide.hpp>

uint64_t PreparatorToHide::getFreeSpaceAfterFiles( const QStringList &filesPaths )
{
    uint64_t totalSize = 0;

    for( const auto &file : filesPaths )
        totalSize += fatManager.getFreeSpaceAfterFile( file );

    return totalSize;
}

uint64_t PreparatorToHide::getSizeToHide( const QStringList &files )
{
    uint64_t size;

    size = getFilesSize( files );
    size += files.size( ) * metadataSize;
    size += sizeof( uint64_t ); // for last filesize 0

    return size;
}

uint64_t PreparatorToHide::getFilesSize( const QStringList &filesPaths )
{
    uint64_t totalSize = 0;

    for( const auto &file : filesPaths )
        totalSize += QFileInfo( file ).size();

    return totalSize;
}

bool PreparatorToHide::sizeTest()
{
    uint64_t sizeToHide, freeSpaceSize;

    taskTree.newTask( "Preparing FAT manager" );
    if( prepareFatManager( partitionDevPath ) == false )
    {
        taskTree.taskFailed();
        return false;
    }
    taskTree.taskSuccess();

    taskTree.newTask( "Calculating space after files on partition" );
    freeSpaceSize = getFreeSpaceAfterFiles( filesOnPartition );
    taskTree.taskSuccess();

    taskTree.newTask( "Calculating size to hide" );
    sizeToHide = getSizeToHide( filesToHide );
    taskTree.taskSuccess();

    taskTree.newTask( "Test: size to hide > free space after files" );
    if( sizeToHide > freeSpaceSize )
    {
        QString reason( "Size to hide (");

        reason += QString::number( sizeToHide );
        reason += ") free space after files (";
        reason += QString ::number( freeSpaceSize );
        reason += "). Not enough space to hide files.";

        taskTree.taskFailed( reason );
        return false;
    }
    taskTree.taskSuccess();

    return true;
}

bool PreparatorToHide::checkPaths()
{
    taskTree.newTask( "Checking paths on partition" );
    if( !Preparator::checkPaths( filesOnPartition ) )
    {
        taskTree.taskFailed();
        return false;
    }
    taskTree.taskSuccess();

    taskTree.newTask( "Checking paths to hide" );
    if( !Preparator::checkPaths( filesToHide ) )
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

bool PreparatorToHide::prepare()
{
    taskTree.newTask( "Checking if paths are correct" );
    if( !checkPaths() )
    {
        taskTree.taskFailed();
        return false;
    }
    taskTree.taskSuccess();

    if( !sizeTest() )
    {
        return false;
    }

    if( !commonPreparation() )
    {
        return false;
    }

    return true;
}
