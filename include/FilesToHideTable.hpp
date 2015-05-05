#ifndef _INCLUDE_FILESTOHIDETABLE_HPP_
#define _INCLUDE_FILESTOHIDETABLE_HPP_

#include <HideSectionFileTable.hpp>

//todo split to cpp file
class FilesToHideTable : public HideSectionFileTable
{
protected:
    void fillFirstColumn( const QString &path );
    void createModel();

public:
    FilesToHideTable() {}
    FilesToHideTable( QTableView *view ) :
        HideSectionFileTable( view )
    {}
    ~FilesToHideTable() {}
};

#endif // _INCLUDE_FILESTOHIDETABLE_HPP_
