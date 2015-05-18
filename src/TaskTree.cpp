#include <TaskTree.hpp>

TaskTree::TaskTree()
{
    createModel();
}

TaskTree::TaskTree( QTreeView *treeView ) :
    view( treeView )
{
    createModel();
}

void TaskTree::createModel()
{
    model = new QStandardItemModel( 0, 0 );
}

void TaskTree::newTask( const QString &taskName )
{
    QStandardItem *item = new QStandardItem( taskName + " ( in progress )" );
    item->setEditable( false );

    if( itemsStack.size() == 0 )
        model->setItem( 0, 0, item );
    else
    {
        auto topItem = itemsStack.top();
        topItem->appendRow( item );
    }

    itemsStack.push( item );
    tasksStack.push( Task( taskName ) );
}

void TaskTree::init( QTreeView *treeView )
{
    view = treeView;
    createModel();
    view->setModel( model );
    view->header()->hide();

}

void TaskTree::taskSuccess()
{
    Task topTask = tasksStack.top();
    QStandardItem *topItem = itemsStack.top();

    topItem->setText( topTask.getName() + " ( successfully done in " + topTask.getTimeAsQString() + " )" );
    topItem->setForeground( QBrush( Qt::darkGreen ) );

    tasksStack.pop();
    itemsStack.pop();
}

void TaskTree::taskFailed( const QString &reason )
{
    Task topTask = tasksStack.top();
    QStandardItem *topItem = itemsStack.top(),
                  *item;

    topItem->setText( topTask.getName() + " ( failed )" );
    topItem->setForeground( QBrush( Qt::darkRed ) );

    if( reason.length() > 0 )
    {
        item = new QStandardItem( reason );
        item->setEditable( false );
        topItem->appendRow( item );
    }

    tasksStack.pop();
    itemsStack.pop();
}

void TaskTree::addInfo( const QString &info )
{
    QStandardItem *topItem = itemsStack.top(),
                  *item = new QStandardItem( info );

    topItem->appendRow( item );
}
