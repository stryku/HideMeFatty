#include <FileTable.hpp>

FileTable::FileTable( QTableView *view) :
    view( view )
{
     createModel();
}

void FileTable::createModel()
{
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel;
    QStandardItemModel *newModel = new QStandardItemModel( 0, 0 );
    newModel->setHorizontalHeaderItem( fileNameColumnIndex, new QStandardItem( QString( "File Name" ) ) );
    newModel->setHorizontalHeaderItem( fullPathColumnIndex, new QStandardItem( QString( "Full Path" ) ) );

    proxyModel->setSourceModel( newModel );
    view->setModel( proxyModel );
    model = newModel;
}

bool FileTable::canAdd( const QString &path ) const
{
    auto rowCount = model->rowCount();
    auto fileInfo = QFileInfo( path );

    if( !fileInfo.exists() || fileInfo.size() == 0 )
        return false;

    for( size_t i = 0; i < rowCount; ++i )
    {
        if( model->item( i, fullPathColumnIndex )->text() == path )
            return false;
    }

    return true;
}

void FileTable::fillColumns( const QString &path )
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

void FileTable::addFile( const QString &path )
{
    if( !canAdd( path ) )
        return;

    fillColumns( path );
}

void FileTable::addFiles( const QStringList &paths )
{
    for( const auto &path : paths )
        addFile( path );


    view->resizeColumnsToContents();
}

void FileTable::init( QTableView *tableView )
{
    view = tableView;
    view->setEditTriggers( QAbstractItemView::NoEditTriggers );
    initColumnsIndexes();
    createModel();
}

QStringList FileTable::getFullPaths() const
{
    QStringList ret;
    auto rowCount = model->rowCount();

    for(size_t i = 0; i < rowCount; ++i )
        ret << model->item( i, fullPathColumnIndex )->text();

    return ret;
}

void FileTable::deleteSelected()
{
    QItemSelection selection( view->selectionModel()->selection() );

    QList<int> rows;
    foreach( const QModelIndex & index, selection.indexes() ) {
       rows.append( index.row() );
    }

    qSort( rows );

    int prev = -1;
    for( int i = rows.count() - 1; i >= 0; i -= 1 ) {
       int current = rows[i];
       if( current != prev ) {
          model->removeRows( current, 1 );
          prev = current;
       }
    }
}
