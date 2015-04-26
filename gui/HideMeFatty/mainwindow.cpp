#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initPartitionsComboBox();
    initHideInfo();
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
    {
        ui->partitionsComboBox->addItem( QString::fromStdString( i.name ) );
        validParitions.push_back( i );
    }
}

void MainWindow::initFileTables()
{
    fileTables.resize( ENUM_FILETABLE_COUNT );

    fileTables[FILETABLE_FILES_ON_PARTITION] = std::make_shared<FilesOnPartitionTable>();
    fileTables[FILETABLE_FILES_TO_HIDE] = std::make_shared<FilesToHideTable>();

    fileTables[FILETABLE_FILES_ON_PARTITION]->init( this, ui->tableViewHidFileOnPartition );
    fileTables[FILETABLE_FILES_TO_HIDE]->init( this, ui->tableViewHideFilesToHide );
}

void MainWindow::initHideInfo()
{
    ui->labFreeSpace->setText( "Total free space: " + QString::number( hideInfo.freeSpace ) );
    ui->labSizeToHide->setText( "Total size to hide: " + QString::number( hideInfo.sizeToHide ) );
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
                                  std::function<void( const QFile& )> functionOnFile,
                                  const QString &caption,
                                  const QString &dir )
{
    auto filePaths = QFileDialog::getOpenFileNames( this,
                                                    caption,
                                                    dir );

    for( const auto &filePath : filePaths )
    {
        QFile file( filePath );
        if( file.open( QIODevice::ReadOnly ) )
        {
            fileTables[tableId]->addFile( filePath );
            functionOnFile( file );
            file.close();
        }
    }
}

void MainWindow::newFileOnPartition( const QFile &file )
{
    AdvancedFileInfo info( file,
                           hideInfo.partitionInfo.clusterSize );

    hideInfo.freeSpace += info.freeSpaceAfterFile;
    ui->labFreeSpace->setText( "Total free space: " + QString::number( hideInfo.freeSpace ) );
}

void MainWindow::newFileToHide( const QFile &file )
{
    hideInfo.sizeToHide += file.size();
    ui->labSizeToHide->setText( "Total size to hide: " + QString::number( hideInfo.sizeToHide ) );
}

void MainWindow::on_addFilesOnPartitionButton_clicked()
{
    auto functionToCallOnFile = std::bind( &MainWindow::newFileOnPartition,
                                           this,
                                           std::placeholders::_1);

    addFilesToTable( FILETABLE_FILES_ON_PARTITION,
                     functionToCallOnFile,
                     "Select files on partition",
                     QString::fromStdString( hideInfo.partitionInfo.mediaPath ) );
}


void MainWindow::on_pushButton_2_clicked()
{
    auto functionToCallOnFile = std::bind( &MainWindow::newFileToHide,
                                           this,
                                           std::placeholders::_1 );

    addFilesToTable( FILETABLE_FILES_TO_HIDE,
                     functionToCallOnFile,
                     "Select files to hide" );
}

void MainWindow::on_partitionsComboBox_currentIndexChanged(int index)
{
    if( index == 0 )
        ui->toolBoxHide->setItemText( 0, "Step 1: Select partition (status: waiting for selecting partition)" );
    else
    {
        hideInfo.partitionInfo = validParitions[index - 1];
        hideInfo.partitionInfo.initClusterSize();
        dynamic_cast< FilesOnPartitionTable*>( fileTables[FILETABLE_FILES_ON_PARTITION].get() )->setFsClusterSize( hideInfo.partitionInfo.clusterSize );
        ui->toolBoxHide->setItemText(0, "Step 1: Select partition (status: ready)");
    }
}
