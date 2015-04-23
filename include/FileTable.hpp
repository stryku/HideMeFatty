#ifndef _INCLUDE_FILESTABLE_HPP_
#define _INCLUDE_FILESTABLE_HPP_


#include <QTableView>
#include <QtGui>

#include <boost/filesystem.hpp>

class QMainWindow;

namespace fs = boost::filesystem;

class FileTable
{
private:
    QTableView *view;
    QStandardItemModel *model;

    void createModel( QMainWindow *mainWindow )
    {
        QStandardItemModel *newModel = new QStandardItemModel( 0, 0, mainWindow );
        newModel->setHorizontalHeaderItem( 0, new QStandardItem( QString( "Size" )));
        newModel->setHorizontalHeaderItem( 1, new QStandardItem( QString( "File Path" ) ) );

        view->setModel( newModel );
        model = newModel;
    }

public:
    FileTable() {}
    FileTable( QTableView *view, QMainWindow *mainWindow ) : view( view )
    {
         createModel( mainWindow );
    }
    ~FileTable() {}

    void addFile( const QString &path )
    {
        auto fileSize = fs::file_size( path.toStdString() );
        auto rowCount = model->rowCount();

        model->appendRow(new QStandardItem());

        model->setItem( rowCount,
                        0,
                        new QStandardItem( QString::number( fileSize ) ) );
        model->setItem( rowCount,
                        1,
                        new QStandardItem( path ) );

    }

    void init( QMainWindow *mainWindow, QTableView *tableView )
    {
        view = tableView;
        createModel( mainWindow );
    }

};

#endif // _INCLUDE_FILESTABLE_HPP_
