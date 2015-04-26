#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QFileDialog>

#include <vector>
#include <string>
#include <functional>
#include <memory>

#include <PartitionFinder.hpp>
#include <Fat32Manager.hpp>
#include <FileTable.hpp>
#include <FilesOnPartitionTable.hpp>
#include <FilesToHideTable.hpp>

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

    void on_pushButton_2_clicked();

    void on_partitionsComboBox_currentIndexChanged(int index);

    void on_pushButton_3_clicked();

private:
    enum EnumFileTable
    {
        FILETABLE_FILES_ON_PARTITION,
        FILETABLE_FILES_TO_HIDE,

        ENUM_FILETABLE_COUNT
    };

    struct HideInfo
    {
        size_t sizeToHide, freeSpace;
        PartitionInfo partitionInfo;

        HideInfo() : sizeToHide( 0 ), freeSpace( 0 ) {}
    }hideInfo;

    Ui::MainWindow *ui;
    QVector<std::shared_ptr<FileTable>> fileTables;
    QVector<PartitionInfo> validParitions;
    Fat32Manager fatManager;

    void initPartitionsComboBox();
    void initFileTables();
    void initHideInfo();

    void addFilesToTable( EnumFileTable tableId,
                          std::function<void( const QFile& )> functionOnFile,
                          const QString &caption = QString(),
                          const QString &dir = QString() );
    std::vector<PartitionInfo> getFat32Partitions();

    void newFileOnPartition( const QFile &file );
    void newFileToHide( const QFile &file );

};

#endif // MAINWINDOW_H
