#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>

#include <vector>
#include <string>

#include <PartitionFinder.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_addFilesOnPartitionButton_clicked();

private:
    Ui::MainWindow *ui;

    void initPartitionsComboBox();
    void initTableViews();
    std::vector<PartitionInfo> getFat32Partitions();
    void initTableView( QTableView *tableView );

};

#endif // MAINWINDOW_H
