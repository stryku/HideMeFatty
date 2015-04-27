#ifndef _INCLUDE_FILESONPARTITIONTABLE_HPP_
#define _INCLUDE_FILESONPARTITIONTABLE_HPP_

#include <FileTable.hpp>
#include <Structs.h>

class HideFilesOnPartitionTable : public FileTable
{
private:
    size_t _fsClusterSize;
public:
    HideFilesOnPartitionTable() {}
    HideFilesOnPartitionTable( QTableView *view,
                           QMainWindow *mainWindow ) :
        FileTable( view, mainWindow )
    {}
    ~HideFilesOnPartitionTable() {}

    void fillFirstColumn( const QString &path, const size_t row )
    {
        AdvancedFileInfo info( path, _fsClusterSize );

        auto freeSpace = info.freeSpaceAfterFile;

        model->setItem( row,
                        firstColumn,
                        new QStandardItem( QString::number( freeSpace ) ) );
    }

    void setFsClusterSize( const size_t fsClusterSize )
    {
        _fsClusterSize = fsClusterSize;
    }
};

#endif // _INCLUDE_FILESONPARTITIONTABLE_HPP_
