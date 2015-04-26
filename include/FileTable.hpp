#ifndef _INCLUDE_FILESTABLE_HPP_
#define _INCLUDE_FILESTABLE_HPP_


#include <QTableView>
#include <QtGui>
#include <QFileInfo>
#include <QStringList>

#include <boost/filesystem.hpp>

#include <pathOperations.hpp>

class QMainWindow;

namespace fs = boost::filesystem;
using namespace pathOperations;

class FileTable
{
protected:
    static const size_t firstColumn = 0;
    static const size_t fileNameColumn = 1;
    static const size_t fullPathColumn = 2;

    QTableView *view;
    QStandardItemModel *model;

private:

    void createModel( QMainWindow *mainWindow )
    {
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel;
        QStandardItemModel *newModel = new QStandardItemModel( 0, 0, mainWindow );
        newModel->setHorizontalHeaderItem( 0, new QStandardItem( QString( "Size" )));
        newModel->setHorizontalHeaderItem( 1, new QStandardItem( QString( "File Name" ) ) );
        newModel->setHorizontalHeaderItem( 2, new QStandardItem( QString( "Full Path" ) ) );

        proxyModel->setSourceModel( newModel );
        view->setModel( proxyModel );
        model = newModel;
    }

public:
    FileTable() {}
    FileTable( QTableView *view,
               QMainWindow *mainWindow ) :
        view( view )
    {
         createModel( mainWindow );
    }
    virtual ~FileTable() {}

    virtual void fillFirstColumn( const QString &path, const size_t row ) = 0;

    bool canAdd( const QString &path ) const
    {
        auto rowCount = model->rowCount();

        for(size_t i = 0; i < rowCount; ++i )
        {
            if( model->item( i, 2 )->text() == path )
                return false;
        }

        return true;
    }

    void addFile( const QString &path )
    {
        if( !canAdd( path ) )
            return;

        auto rowCount = model->rowCount();
        QFileInfo fileInfo( path );
        auto fileName = fileInfo.fileName();

        model->appendRow(new QStandardItem());

        model->setItem( rowCount,
                        fileNameColumn,
                        new QStandardItem( fileName ) );

        model->setItem( rowCount,
                        fullPathColumn,
                        new QStandardItem( path ) );

        fillFirstColumn( path, rowCount );

        view->resizeColumnsToContents();
    }

    void init( QMainWindow *mainWindow, QTableView *tableView )
    {
        view = tableView;
        view->setEditTriggers( QAbstractItemView::NoEditTriggers );
        createModel( mainWindow );
    }

    void sortByCollumn( size_t collumn, Qt::SortOrder sortOrder )
    {
        view->sortByColumn( collumn, sortOrder );
    }

    QStringList getFullPaths() const
    {
        QStringList ret;
        auto rowCount = model->rowCount();

        for(size_t i = 0; i < rowCount; ++i )
            ret << model->item( i, fullPathColumn )->text();

        return ret;
    }
};

#endif // _INCLUDE_FILESTABLE_HPP_
