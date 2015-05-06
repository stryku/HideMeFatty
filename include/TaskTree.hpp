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
    
    void createModel()
    {
        model = new QStandardItemModel( 0, 0 );
    }
    
public:
    TaskTree()
    {
        createModel();
    }
    TaskTree( QTreeView *treeView ) :
        view( treeView )
    {
        createModel();
    }

    ~TaskTree() {}

    void newTask( const QString &taskName )
    {
        QStandardItem *item = new QStandardItem( taskName + " ( in progress )" );

        if( itemsStack.size() == 0 )
            model->setItem( 0, 0, item );
        else
        {
            auto topItem = itemsStack.top();
            size_t row = topItem->column();
            topItem->appendRow( item );
        }

        itemsStack.push( item );
        tasksStack.push( Task( taskName ) );
    }

    void init( QTreeView *treeView )
    {
        view = treeView;
        createModel();
        view->setModel( model );
        view->header()->hide();

    }

    void endOfTask()
    {
        Task topTask = tasksStack.top();
        QStandardItem *topItem = itemsStack.top();

        topItem->setText( topTask.getNameWithTime() );

        tasksStack.pop();
        itemsStack.pop();
    }

};

#endif // _INCLUDE_TASKTREE_HPP_
