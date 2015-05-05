#include <FilesToHideTable.hpp>

void FilesToHideTable::fillFirstColumn( const QString &path )
{
    auto fileSize = QFile( path ).size();

    model->setItem( model->rowCount() - 1,
                    firstColumnIndex,
                    new QStandardItem( QString::number( fileSize ) ) );
}

void FilesToHideTable::createModel()
{
    FileTable::createModel();
    model->setHorizontalHeaderItem( firstColumnIndex, new QStandardItem( QString( "File size" ) ) );
}
