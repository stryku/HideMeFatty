#ifndef _INCLUDE_TASKTREE_HPP_
#define _INCLUDE_TASKTREE_HPP_

#include <QStandardItemModel>
#include <QTreeView>
#include <QHeaderView>

#include <stack>

#include <Task.hpp>

class TaskTree
{
private:
    QStandardItemModel *model;
    QTreeView *view;

    std::stack<Task> tasksStack;
    std::stack<QStandardItem*> itemsStack;
    
    void createModel();
    
public:
    TaskTree();
    TaskTree( QTreeView *treeView );

    ~TaskTree() {}

    void newTask( const QString &taskName );

    void init( QTreeView *treeView );

    void taskSuccess();

    void taskFailed( const QString &reason = QString() );

    void addInfo( const QString &info );

};

#endif // _INCLUDE_TASKTREE_HPP_
