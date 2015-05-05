#include <HideFilesOnPartitionTable.hpp>

void HideFilesOnPartitionTable::fillFirstColumn( const QString &path )
{
    AdvancedFileInfo info( path, _fsClusterSize );

    auto freeSpace = info.freeSpaceAfterFile;

    model->setItem( model->rowCount() - 1,
                    firstColumnIndex,
                    new QStandardItem( QString::number( freeSpace ) ) );
}
void HideFilesOnPartitionTable::createModel()
{
    FileTable::createModel();
    model->setHorizontalHeaderItem( firstColumnIndex, new QStandardItem( QString( "Free space" ) ) );
}

void HideFilesOnPartitionTable::setFsClusterSize( const size_t fsClusterSize )
{
    _fsClusterSize = fsClusterSize;
}
