#ifndef _INCLUDE_FILESONPARTITIONTABLE_HPP_
#define _INCLUDE_FILESONPARTITIONTABLE_HPP_

#include <HideSectionFileTable.hpp>
#include <Structs.h>

class HideFilesOnPartitionTable : public HideSectionFileTable
{
protected:
    void fillFirstColumn( const QString &path );
    void createModel();

private:
    size_t _fsClusterSize;

public:
    HideFilesOnPartitionTable() {}
    HideFilesOnPartitionTable( QTableView *view ) :
        HideSectionFileTable( view )
    {}
    ~HideFilesOnPartitionTable() {}

    void setFsClusterSize( const size_t fsClusterSize );

};

#endif // _INCLUDE_FILESONPARTITIONTABLE_HPP_
