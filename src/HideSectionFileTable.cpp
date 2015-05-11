#include <HideSectionFileTable.hpp>

void HideSectionFileTable::initColumnsIndexes()
{
    fileNameColumnIndex = 1;
    fullPathColumnIndex = 2;
}

void HideSectionFileTable::fillColumns( const QString &path )
{
    FileTable::fillColumns( path );

    fillFirstColumn( path );
}

size_t HideSectionFileTable::getAcumulatedFirstColumn()
{
    size_t ret = 0;
    auto rowCount = model->rowCount();

    for( size_t i = 0; i < rowCount; ++i )
        ret += model->item( i, firstColumnIndex )->data( Qt::DisplayRole ).toUInt();

    return ret;
}
