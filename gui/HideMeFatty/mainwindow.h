#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QFileDialog>

#include <vector>
#include <string>
#include <functional>
#include <memory>

#include <Fat32Manager.hpp>
#include <PartitionFinder.hpp>
#include <FileTable.hpp>
#include <HideFilesOnPartitionTable.hpp>
#include <FilesToHideTable.hpp>
#include <RestoreFilesOnPartitionTable.hpp>

namespace Ui {
class MainWindow;
}

typedef std::function<void( const QFile& )> FunctionOnFile;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_addFilesOnPartitionButton_clicked();

    void on_pushButton_2_clicked();

    void on_comboBoxHidePartitions_currentIndexChanged(int index);

    void on_pushButton_3_clicked();

    void on_pushButtonRestAddFilesOnPartition_clicked();

    void on_comboBoxRestPartitions_currentIndexChanged(int index);

    void on_pushButtonSelectFolderToStore_clicked();

    void on_pushButtonRestoreFiles_clicked();

private:
    enum EnumFileTable
    {
        FILETABLE_HIDE_FILES_ON_PARTITION,
        FILETABLE_FILES_TO_HIDE,
        FILETABLE_RESTORE_FILES_ON_PARTITION,

        ENUM_FILETABLE_COUNT
    };

    Ui::MainWindow *ui;
    QVector<std::shared_ptr<FileTable>> fileTables;
    QVector<PartitionInfo> validParitions;
    Fat32Manager fatManager;
    PartitionInfo hideSelectedPartition,
                  restoreSelectedPartition;

    void initPartitionsComboBoxes();
    void initFileTables();
    void initHideInfo();

    void addFilesToTable( EnumFileTable tableId,
                          const QString &caption = QString(),
                          const QString &dir = QString() );
    std::vector<PartitionInfo> getFat32Partitions();
};

#endif // MAINWINDOW_H
