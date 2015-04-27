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

    void fillColumns( const QString &path )
    {
        FileTable::fillColumns( path );

        fillFirstColumn( path );
    }

public:
    HideSectionFileTable() {}
    HideSectionFileTable( QTableView *view,
                          QMainWindow *mainWindow ) :
        FileTable( view, mainWindow )
    {}
    virtual ~HideSectionFileTable() {}

    size_t getAcumulatedFirstColumn()
    {
        size_t ret = 0;
        auto rowCount = model->rowCount();

        for(size_t i = 0; i < rowCount; ++i )
            ret += model->item( i, firstColumnIndex )->data( Qt::DisplayRole ).toUInt();

        return ret;
    }
};

#endif // _INCLUDE_HIDESECTIONFILETABLE_HPP_
