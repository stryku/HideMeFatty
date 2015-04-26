#ifndef _INCLUDE_FILESTOHIDETABLE_HPP_
#define _INCLUDE_FILESTOHIDETABLE_HPP_

#include <FileTable.hpp>

class FilesToHideTable : public FileTable
{
public:
    FilesToHideTable() {}
    FilesToHideTable( QTableView *view,
                           QMainWindow *mainWindow ) :
        FileTable( view, mainWindow )
    {}
    ~FilesToHideTable() {}

    void fillFirstColumn( const QString &path, const size_t row )
    {
        auto fileSize = QFile( path ).size();

        model->setItem( row,
                        firstColumn,
                        new QStandardItem( QString::number( fileSize ) ) );
    }
};

#endif // _INCLUDE_FILESTOHIDETABLE_HPP_
