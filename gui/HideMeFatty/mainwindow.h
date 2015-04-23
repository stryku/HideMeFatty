#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QFileDialog>

#include <vector>
#include <string>

#include <PartitionFinder.hpp>
#include <FileTable.hpp>

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
    enum EnumFileTable
    {
        FILETABLE_FILES_ON_PARTITION,
        FILETABLE_FILES_TO_HIDE,

        ENUM_FILETABLE_COUNT
    };

    Ui::MainWindow *ui;
    QVector<FileTable> fileTables;

    void initPartitionsComboBox();
    void initFileTables();
    void addFilesToTable( EnumFileTable tableId );
    std::vector<PartitionInfo> getFat32Partitions();

};

#endif // MAINWINDOW_H
