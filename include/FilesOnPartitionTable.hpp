#ifndef _INCLUDE_FILESONPARTITIONTABLE_HPP_
#define _INCLUDE_FILESONPARTITIONTABLE_HPP_

#include <FileTable.hpp>
#include <Structs.h>

class FilesOnPartitionTable : public FileTable
{
private:
    size_t _fsClusterSize;
public:
    FilesOnPartitionTable() {}
    FilesOnPartitionTable( QTableView *view,
                           QMainWindow *mainWindow ) :
        FileTable( view, mainWindow )
    {}
    ~FilesOnPartitionTable() {}

    void fillFirstColumn( const QString &path, const size_t row )
    {
        auto freeSpace = AdvancedFileInfo( path, _fsClusterSize ).freeSpaceAfterFile;

        model->setItem( row,
                        0,
                        new QStandardItem( QString::number( freeSpace ) ) );
    }

    void setFsClusterSize( const size_t fsClusterSize )
    {
        _fsClusterSize = fsClusterSize;
    }
};

#endif // _INCLUDE_FILESONPARTITIONTABLE_HPP_
