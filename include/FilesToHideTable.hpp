#ifndef _INCLUDE_FILESTOHIDETABLE_HPP_
#define _INCLUDE_FILESTOHIDETABLE_HPP_

#include <HideSectionFileTable.hpp>

//todo split to cpp file
class FilesToHideTable : public HideSectionFileTable
{
public:
    FilesToHideTable() {}
    FilesToHideTable( QTableView *view ) :
        HideSectionFileTable( view )
    {}
    ~FilesToHideTable() {}

protected:
    void fillFirstColumn( const QString &path)
    {
        auto fileSize = QFile( path ).size();

        model->setItem( model->rowCount() - 1,
                        firstColumnIndex,
                        new QStandardItem( QString::number( fileSize ) ) );
    }

    void createModel()
    {
        FileTable::createModel();
        model->setHorizontalHeaderItem( firstColumnIndex, new QStandardItem( QString( "File size" ) ) );
    }
};

#endif // _INCLUDE_FILESTOHIDETABLE_HPP_
