#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QFileDialog>

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

void MainWindow::initTableViews()
{
    QStandardItemModel *model = new QStandardItemModel(2, 2, this);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Size")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("File Path")));


    ui->tableViewHidFileOnPartition->setModel( new QStandardItemModel(*model) );
    ui->tableViewHideFilesToHide->setModel(model);
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

