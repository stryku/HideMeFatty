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
    virtual void createModel();

public:
    FileTable() {}
    FileTable( QTableView *view );
    virtual ~FileTable() {}

    bool canAdd( const QString &path ) const;
    virtual void fillColumns( const QString &path );
    void addFile( const QString &path );
    void init( QTableView *tableView );
    QStringList getFullPaths() const;
};

#endif // _INCLUDE_FILESTABLE_HPP_
