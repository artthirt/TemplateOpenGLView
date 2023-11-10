#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QDir dir;
    mDefaultDir = dir.homePath();

    auto w = QWidget::createWindowContainer(&mView);
    ui->vlCntr->addWidget(w);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    auto str = QFileDialog::getOpenFileName(nullptr, "Open Image...", mDefaultDir,
                                            "Images (*.bmp *.png *.jpg *.jpeg *.tif *.tiff *.pgm *.ppm)");
    if(!str.isEmpty()){
        mView.setImg(str);
    }
}

