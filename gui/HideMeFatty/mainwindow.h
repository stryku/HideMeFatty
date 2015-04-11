#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <string>

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
    std::vector<std::string> getFat32Partitions();

};

#endif // MAINWINDOW_H
