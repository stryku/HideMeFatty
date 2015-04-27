#ifndef _INCLUDE_HIDESECTIONFILETABLE_HPP_
#define _INCLUDE_HIDESECTIONFILETABLE_HPP_

#include <FileTable.hpp>

class HideSectionFileTable : public FileTable
{
protected:
    static const size_t firstColumnIndex = 0;

    virtual void fillFirstColumn( const QString & ) = 0;

    void initColumnsIndexes()
    {
        fileNameColumnIndex = 1;
        fullPathColumnIndex = 2;
    }

public:
    HideSectionFileTable() {}
    HideSectionFileTable( QTableView *view,
                          QMainWindow *mainWindow ) :
        FileTable( view, mainWindow )
    {}
    virtual ~HideSectionFileTable() {}

    virtual void fillColumns( const QString &path )
    {
        FileTable::fillColumns( path );

        fillFirstColumn( path );
    }
};

#endif // _INCLUDE_HIDESECTIONFILETABLE_HPP_
