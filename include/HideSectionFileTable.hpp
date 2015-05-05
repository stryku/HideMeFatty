#ifndef _INCLUDE_HIDESECTIONFILETABLE_HPP_
#define _INCLUDE_HIDESECTIONFILETABLE_HPP_

#include <FileTable.hpp>

class HideSectionFileTable : public FileTable
{
protected:
    static const size_t firstColumnIndex = 0;

    virtual void fillFirstColumn( const QString & ) = 0;
    void initColumnsIndexes();
    void fillColumns( const QString &path );

public:
    HideSectionFileTable() {}
    HideSectionFileTable( QTableView *view ) :
        FileTable( view )
    {}
    virtual ~HideSectionFileTable() {}

    size_t getAcumulatedFirstColumn();
};

#endif // _INCLUDE_HIDESECTIONFILETABLE_HPP_
