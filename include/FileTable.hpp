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
    size_t fileNameColumnIndex;
    size_t fullPathColumnIndex;

    QTableView *view;
    QStandardItemModel *model;

    virtual void initColumnsIndexes() = 0;
    virtual void createModel( QMainWindow *mainWindow )
    {
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel;
        QStandardItemModel *newModel = new QStandardItemModel( 0, 0, mainWindow );
        newModel->setHorizontalHeaderItem( fileNameColumnIndex, new QStandardItem( QString( "File Name" ) ) );
        newModel->setHorizontalHeaderItem( fullPathColumnIndex, new QStandardItem( QString( "Full Path" ) ) );

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

    bool canAdd( const QString &path ) const
    {
        auto rowCount = model->rowCount();

        for( size_t i = 0; i < rowCount; ++i )
        {
            if( model->item( i, fullPathColumnIndex )->text() == path )
                return false;
        }

        return true;
    }

    virtual void fillColumns( const QString &path )
    {
        auto rowCount = model->rowCount();
        QFileInfo fileInfo( path );
        auto fileName = fileInfo.fileName();

        model->appendRow( new QStandardItem() );

        model->setItem( rowCount,
                        fileNameColumnIndex,
                        new QStandardItem( fileName ) );

        model->setItem( rowCount,
                        fullPathColumnIndex,
                        new QStandardItem( path ) );
    }

    void addFile( const QString &path )
    {
        if( !canAdd( path ) )
            return;

        fillColumns( path );

        view->resizeColumnsToContents();
    }

    void init( QMainWindow *mainWindow, QTableView *tableView )
    {
        view = tableView;
        view->setEditTriggers( QAbstractItemView::NoEditTriggers );
        initColumnsIndexes();
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
            ret << model->item( i, fullPathColumnIndex )->text();

        return ret;
    }
};

#endif // _INCLUDE_FILESTABLE_HPP_
