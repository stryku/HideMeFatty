#ifndef _INCLUDE_RESTOREFILESONPARTITIONTABLE_HPP_
#define _INCLUDE_RESTOREFILESONPARTITIONTABLE_HPP_

#include <FileTable.hpp>

class RestoreFilesOnPartitionTable : public FileTable
{
protected:
    void initColumnsIndexes()
    {
        fileNameColumnIndex = 0;
        fullPathColumnIndex = 1;
    }

public:
    RestoreFilesOnPartitionTable() {}
    RestoreFilesOnPartitionTable( QTableView *view,
                          QMainWindow *mainWindow ) :
        FileTable( view, mainWindow )
    {}
    virtual ~RestoreFilesOnPartitionTable() {}
};

#endif // _INCLUDE_RESTOREFILESONPARTITIONTABLE_HPP_
