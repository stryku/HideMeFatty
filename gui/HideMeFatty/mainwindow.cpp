#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <FileHider.hpp>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initPartitionsComboBoxes();
    initHideInfo();
    initFileTables();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initPartitionsComboBoxes()
{
    auto partitions = getFat32Partitions();

    for( const auto &i : partitions)
    {
        ui->comboBoxHidePartitions->addItem( i.name );
        ui->comboBoxRestPartitions->addItem( i.name );
        validParitions.push_back( i );
    }
}

void MainWindow::initFileTables()
{
    fileTables.resize( ENUM_FILETABLE_COUNT );

    fileTables[FILETABLE_HIDE_FILES_ON_PARTITION] = std::make_shared<HideFilesOnPartitionTable>();
    fileTables[FILETABLE_FILES_TO_HIDE] = std::make_shared<FilesToHideTable>();
    fileTables[FILETABLE_RESTORE_FILES_ON_PARTITION] = std::make_shared<RestoreFilesOnPartitionTable>();

    fileTables[FILETABLE_HIDE_FILES_ON_PARTITION]->init( ui->tableViewHidFileOnPartition );
    fileTables[FILETABLE_FILES_TO_HIDE]->init( ui->tableViewHideFilesToHide );
    fileTables[FILETABLE_RESTORE_FILES_ON_PARTITION]->init( ui->tableViewRestoreFilesOnPartition );
}

void MainWindow::initHideInfo()
{
    ui->labFreeSpace->setText( "Total free space: 0" );
    ui->labSizeToHide->setText( "Total size to hide: 0" );
}

std::vector<PartitionInfo> MainWindow::getFat32Partitions()
{
    std::vector<PartitionInfo> ret;

    auto partitions = PartitionFinder().getMountedPartitions();

    for( const auto &i : partitions )
    {
        if( i.filesystem == "vfat" )
            ret.push_back( i );
    }

    return ret;
}

void MainWindow::addFilesToTable( EnumFileTable tableId,
                                  const QString &caption,
                                  const QString &dir )
{
    auto filePaths = QFileDialog::getOpenFileNames( this,
                                                    caption,
                                                    dir );

    for( const auto &filePath : filePaths )
    {
        QFile file( filePath );
        if( file.open( QIODevice::ReadOnly ) && file.size() > 0 )
        {
            fileTables[tableId]->addFile( filePath );
            file.close();
        }
    }
}

void MainWindow::on_addFilesOnPartitionButton_clicked()
{
    size_t freeSpace;
    HideSectionFileTable *table;

    addFilesToTable( FILETABLE_HIDE_FILES_ON_PARTITION,
                     "Select files on partition",
                     hideSelectedPartition.mediaPath );

    table = dynamic_cast<HideSectionFileTable*>( fileTables[FILETABLE_HIDE_FILES_ON_PARTITION].get() );

    freeSpace = table->getAcumulatedFirstColumn();

    ui->labFreeSpace->setText( QString::number( freeSpace ) );

    ui->labFreeSpace->setText( "Total free space: " + QString::number( freeSpace ) );
}

//TODO ALMOST SMART LABEL
void MainWindow::on_pushButton_2_clicked()
{
    size_t sizeToHide;
    HideSectionFileTable *table;

    addFilesToTable( FILETABLE_FILES_TO_HIDE,
                     "Select files to hide",
                     "." );

    table = dynamic_cast<HideSectionFileTable*>( fileTables[FILETABLE_FILES_TO_HIDE].get() );

    sizeToHide = table->getAcumulatedFirstColumn();

    ui->labSizeToHide->setText( "Total size to hide: " + QString::number( sizeToHide ) );
}

void MainWindow::on_comboBoxHidePartitions_currentIndexChanged(int index)
{
    if( index > 0 )
    {
        HideFilesOnPartitionTable *tablePtr;

        tablePtr = dynamic_cast< HideFilesOnPartitionTable*>( fileTables[FILETABLE_HIDE_FILES_ON_PARTITION].get() );

        hideSelectedPartition = validParitions[index - 1];
        hideSelectedPartition.initClusterSize();

        tablePtr->setFsClusterSize( hideSelectedPartition.clusterSize );
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    auto filesOnPartition = fileTables[FILETABLE_HIDE_FILES_ON_PARTITION]->getFullPaths(),
            filesToHide = fileTables[FILETABLE_FILES_TO_HIDE]->getFullPaths();

    auto partitionDevPath = hideSelectedPartition.devicePath,
            partitionMediaPath = hideSelectedPartition.mediaPath;

    FileHider fileHider;

    if( fileHider.hideFiles( filesOnPartition,
                             partitionMediaPath,
                             filesToHide,
                             partitionDevPath ) )
    {
        QMessageBox::information(this, "ok", "ok");
    }
    else
    {
        QMessageBox::information(this, "chuja", "chuja");

    }
}

void MainWindow::on_pushButtonRestAddFilesOnPartition_clicked()
{
    addFilesToTable( FILETABLE_RESTORE_FILES_ON_PARTITION,
                     "Select files",
                     restoreSelectedPartition.mediaPath );
}

void MainWindow::on_comboBoxRestPartitions_currentIndexChanged(int index)
{
    if( index > 0 )
    {
        restoreSelectedPartition = validParitions[index - 1];
    }
}

void MainWindow::on_pushButtonSelectFolderToStore_clicked()
{
    auto selectedFolder = QFileDialog::getExistingDirectory();

    ui->labelSelectedFolderToStore->setText( selectedFolder );
}

void MainWindow::on_pushButtonRestoreFiles_clicked()
{
    auto filesOnPartition = fileTables[FILETABLE_RESTORE_FILES_ON_PARTITION]->getFullPaths();

    auto partitionDevPath = restoreSelectedPartition.devicePath,
            partitionMediaPath = restoreSelectedPartition.mediaPath;
    auto pathToStore = ui->labelSelectedFolderToStore->text();

    FileHider fileHider;

    if( fileHider.restoreMyFiles( filesOnPartition,
                                  partitionMediaPath,
                                  partitionDevPath,
                                  pathToStore ) )
    {
        QMessageBox::information(this, "ok", "ok");
    }
    else
    {
        QMessageBox::information(this, "chuja", "chuja");

    }
}
