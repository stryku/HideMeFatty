#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initPartitionsComboBox();
    initFileTables();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initPartitionsComboBox()
{
    auto partitions = getFat32Partitions();

    for( const auto &i : partitions)
        ui->partitionsComboBox->addItem( QString::fromStdString( i.name ) );
}

void MainWindow::initFileTables()
{
    fileTables.resize( ENUM_FILETABLE_COUNT );

    fileTables[FILETABLE_FILES_ON_PARTITION].init( this, ui->tableViewHidFileOnPartition );
    fileTables[FILETABLE_FILES_TO_HIDE].init( this, ui->tableViewHideFilesToHide );
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

void MainWindow::addFilesToTable( EnumFileTable tableId )
{
    auto filePaths = QFileDialog::getOpenFileNames();

    for( const auto &filePath : filePaths )
        fileTables[tableId].addFile( filePath );
}

void MainWindow::on_addFilesOnPartitionButton_clicked()
{
    addFilesToTable( FILETABLE_FILES_ON_PARTITION );
}

