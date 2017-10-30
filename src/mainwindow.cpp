#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "FilePath.h"
#include "WorkerThreads.h"
#include <QtGlobal>
#include <QDebug>
#include <QVector>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
}

MainWindow::~MainWindow(){  delete ui;  }
