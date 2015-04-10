#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initPartitionsComboBox();
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

std::vector<std::string> MainWindow::getFat32Partitions()
{
    std::vector<std::string> ret;

    ret.push_back("siema");

    return ret;
}
