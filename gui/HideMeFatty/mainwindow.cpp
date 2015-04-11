#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QtGui>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initPartitionsComboBox();
    initTableViews();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initPartitionsComboBox()
{
    std::vector<std::string> partitions;

    partitions = getFat32Partitions();

    for( const auto &i : partitions)
        ui->partitionsComboBox->addItem(QString::fromStdString(i));
}

void MainWindow::initTableView( QTableView *tableView )
{
    QStandardItemModel *model = new QStandardItemModel(0, 0, this);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Size")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("File Path")));

    tableView->setModel( model );
}

void MainWindow::initTableViews()
{
    initTableView( ui->tableViewHideFilesToHide );
    initTableView( ui->tableViewHidFileOnPartition );
}

std::vector<std::string> MainWindow::getFat32Partitions()
{
    std::vector<std::string> ret;

    ret.push_back("siema");

    return ret;
}

void MainWindow::on_addFilesOnPartitionButton_clicked()
{
    auto fileNames = QFileDialog::getOpenFileNames();
}

