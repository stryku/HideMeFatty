#ifndef _INCLUDE_FILESONPARTITIONTABLE_HPP_
#define _INCLUDE_FILESONPARTITIONTABLE_HPP_

#include <HideSectionFileTable.hpp>
#include <Structs.h>

//todo split to cpp file
class HideFilesOnPartitionTable : public HideSectionFileTable
{
private:
    size_t _fsClusterSize;
public:
    HideFilesOnPartitionTable() {}
    HideFilesOnPartitionTable( QTableView *view,
                               QMainWindow *mainWindow ) :
        HideSectionFileTable( view, mainWindow )
    {}
    ~HideFilesOnPartitionTable() {}

protected:
    void fillFirstColumn( const QString &path )
    {
        AdvancedFileInfo info( path, _fsClusterSize );

        auto freeSpace = info.freeSpaceAfterFile;

        model->setItem( model->rowCount() - 1,
                        firstColumnIndex,
                        new QStandardItem( QString::number( freeSpace ) ) );
    }
    void createModel( QMainWindow *mainWindow )
    {
        FileTable::createModel( mainWindow );
        model->setHorizontalHeaderItem( firstColumnIndex, new QStandardItem( QString( "Free space" ) ) );
    }

public:
    void setFsClusterSize( const size_t fsClusterSize )
    {
        _fsClusterSize = fsClusterSize;
    }
};

#endif // _INCLUDE_FILESONPARTITIONTABLE_HPP_
