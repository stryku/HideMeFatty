#ifndef _INCLUDE_FILESTABLE_HPP_
#define _INCLUDE_FILESTABLE_HPP_


#include <QTableView>
#include <QtGui>
#include <QFileInfo>

#include <boost/filesystem.hpp>

#include <pathOperations.hpp>

class QMainWindow;

namespace fs = boost::filesystem;
using namespace pathOperations;

class FileTable
{
protected:
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
    FileTable( QTableView *view, QMainWindow *mainWindow ) : view( view )
    {
         createModel( mainWindow );
    }
    ~FileTable() {}

    virtual void fillFirstColumn( const QString &path ) = 0;

    void addFile( const QString &path )
    {
        auto rowCount = model->rowCount();
        QFileInfo fileInfo( path );
        auto fileName = fileInfo.fileName();

        model->appendRow(new QStandardItem());

        model->setItem( rowCount,
                        1,
                        new QStandardItem( fileName ) );

        model->setItem( rowCount,
                        2,
                        new QStandardItem( path ) );

        fillFirstColumn( path );

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
};

#endif // _INCLUDE_FILESTABLE_HPP_
