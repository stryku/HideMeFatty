#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <FileHider.hpp>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    refreshPartitionsComboBoxes();
    initHideInfo();
    initFileTables();
    initTaskTrees();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshPartitionsComboBoxes()
{
    auto partitions = getFat32Partitions();

    ui->comboBoxHidePartitions->clear();
    ui->comboBoxRestPartitions->clear();
    validParitions.clear();

    ui->comboBoxHidePartitions->addItem( "No partition selected" );
    ui->comboBoxRestPartitions->addItem( "No partition selected" );

    for( const auto &i : partitions)
    {
        ui->comboBoxHidePartitions->addItem( i.name );
        ui->comboBoxRestPartitions->addItem( i.name );
        validParitions.push_back( i );
    }
}

void MainWindow::initTaskTrees()
{
    taskTreeHide.init( ui->treeViewHideTasks );
    taskTreeRestore.init( ui->treeViewRestoreTasks );
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
    auto filesPaths = QFileDialog::getOpenFileNames( this,
                                                    caption,
                                                    dir );

    fileTables[tableId]->addFiles( filesPaths );
}

void MainWindow::on_addFilesOnPartitionButton_clicked()
{
    size_t freeSpace;
    HideSectionFileTable *table;

    if( ui->comboBoxHidePartitions->currentIndex() == 0 )
    {
        QMessageBox::information(
               this,
               tr("Go to step 1"),
               tr("Please select partition in Step 1.") );

        return;
    }

    addFilesToTable( FILETABLE_HIDE_FILES_ON_PARTITION,
                     "Select files on partition",
                     hideSelectedPartition.mediaPath );

    table = dynamic_cast<HideSectionFileTable*>( fileTables[FILETABLE_HIDE_FILES_ON_PARTITION].get() );

    freeSpace = table->getAcumulatedFirstColumn();

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

        try
        {
            hideSelectedPartition.initClusterSize();
        }
        catch( const std::ios_base::failure& e )
        {
            QMessageBox::critical( this,
                                   tr("Error has occured"),
                                   tr("Did you forget to \"sudo chown\" partition device file?") );

            ui->comboBoxHidePartitions->setCurrentIndex( 0 );

            return;
        }

        tablePtr->setFsClusterSize( hideSelectedPartition.clusterSize );
    }
}

bool MainWindow::hideFiles()
{
    auto filesOnPartition = fileTables[FILETABLE_HIDE_FILES_ON_PARTITION]->getFullPaths(),
            filesToHide = fileTables[FILETABLE_FILES_TO_HIDE]->getFullPaths();

    auto partitionDevPath = hideSelectedPartition.devicePath,
            partitionMediaPath = hideSelectedPartition.mediaPath;

    FileHider fileHider( taskTreeHide );

    return fileHider.hideFiles( filesOnPartition,
                                partitionMediaPath,
                                filesToHide,
                                partitionDevPath );
}

void MainWindow::on_pushButtonStartHiding_clicked()
{
    auto filesToHide = fileTables[FILETABLE_FILES_TO_HIDE]->getFullPaths();

    if( filesToHide.size() == 0 )
    {
        QMessageBox::information(
               this,
               tr("Nothing to hide"),
               tr("Please select files to hide in Step 3") );

        return;
    }
    else
    {
        ui->pushButtonStartHiding->setEnabled( false );
        ui->pushButtonStartHiding->setText( "Hiding started. Please be patient..." );

        taskTreeHide.newTask( "Hiding files" );

        if( hideFiles() )
            taskTreeHide.taskSuccess();
        else
            taskTreeHide.taskFailed();

        ui->pushButtonStartHiding->setEnabled( true );
        ui->pushButtonStartHiding->setText( "Start hiding" );
    }
}

void MainWindow::on_pushButtonRestAddFilesOnPartition_clicked()
{
    if( ui->comboBoxRestPartitions->currentIndex() == 0 )
    {
        QMessageBox::information(
               this,
               tr("Go to step 1"),
               tr("Please select partition in Step 1.") );

        return;
    }

    addFilesToTable( FILETABLE_RESTORE_FILES_ON_PARTITION,
                     "Select files",
                     restoreSelectedPartition.mediaPath );
}

void MainWindow::on_comboBoxRestPartitions_currentIndexChanged(int index)
{
    if( index > 0 )
    {
        restoreSelectedPartition = validParitions[index - 1];

        try
        {
            if( !restoreSelectedPartition.isValidFat32() )
            {
                QMessageBox::critical( this,
                                       tr("Error has occured"),
                                       tr("On that partition there is no valid FAT32 filesystem.") );
            }
        }
        catch( const std::ios_base::failure& e )
        {
            QMessageBox::critical( this,
                                   tr("Error has occured"),
                                   tr("Did you forget to \"sudo chown\" partition device file?") );

            ui->comboBoxRestPartitions->setCurrentIndex( 0 );

            return;
        }
    }
}

void MainWindow::on_pushButtonSelectFolderToStore_clicked()
{
    auto selectedFolder = QFileDialog::getExistingDirectory();

    ui->labelSelectedFolderToStore->setText( selectedFolder );
}

bool MainWindow::restoreFiles()
{
    auto filesOnPartition = fileTables[FILETABLE_RESTORE_FILES_ON_PARTITION]->getFullPaths();

    auto partitionDevPath = restoreSelectedPartition.devicePath,
            partitionMediaPath = restoreSelectedPartition.mediaPath;
    auto pathToStore = ui->labelSelectedFolderToStore->text();

    FileHider fileHider( taskTreeRestore );

    ui->pushButtonRestoreFiles->setEnabled( false );
    ui->pushButtonRestoreFiles->setText( "Restoring started. Please be patient..." );

    return fileHider.restoreMyFiles( filesOnPartition,
                                     partitionMediaPath,
                                     partitionDevPath,
                                     pathToStore );

}

void MainWindow::on_pushButtonRestoreFiles_clicked()
{
    auto filesOnPartition = fileTables[FILETABLE_RESTORE_FILES_ON_PARTITION]->getFullPaths();

    if( filesOnPartition.size() == 0 )
    {
        QMessageBox::information( this,
                                  tr("Can't restore"),
                                  tr("Please select files on partition behind which you hid files") );

        ui->comboBoxRestPartitions->setCurrentIndex( 0 );

        return;
    }
    else
    {
        ui->pushButtonRestoreFiles->setEnabled( false );
        ui->pushButtonRestoreFiles->setText( "Restoring started. Please be patient..." );

        taskTreeRestore.newTask( "Restoring files" );

        if( restoreFiles() )
            taskTreeRestore.taskSuccess();
        else
            taskTreeRestore.taskFailed();

        ui->pushButtonStartHiding->setEnabled( true );
        ui->pushButtonStartHiding->setText( "Restore files" );
    }
}

void MainWindow::on_pushButtonRefreshHidePartitions_clicked()
{
    refreshPartitionsComboBoxes();
}

void MainWindow::on_pushButtonRefreshRestorePartitions_clicked()
{
    refreshPartitionsComboBoxes();
}

void MainWindow::deleteFromHideSectionTable( EnumFileTable tableId,
                                             QLabel *connectedLabel,
                                             QString labelPrefix )
{
    size_t firstColumnValue;
    HideSectionFileTable *table;

    table = dynamic_cast<HideSectionFileTable*>( fileTables[tableId].get() );

    table->deleteSelected();

    firstColumnValue = table->getAcumulatedFirstColumn();

    connectedLabel->setText( labelPrefix + QString::number( firstColumnValue ) );
}

void MainWindow::on_pushButtonHideDeleteFilesOnPartition_clicked()
{
   deleteFromHideSectionTable( FILETABLE_HIDE_FILES_ON_PARTITION,
                               ui->labFreeSpace,
                               "Total free space: " );
}

void MainWindow::on_pushButtonHideDeleteFilesToHide_clicked()
{

    deleteFromHideSectionTable( FILETABLE_FILES_TO_HIDE,
                                ui->labSizeToHide,
                                "Total size to hide: " );
}

void MainWindow::on_pushButtonRestoreFilesOnPartition_clicked()
{
    fileTables[FILETABLE_RESTORE_FILES_ON_PARTITION]->deleteSelected();
}
