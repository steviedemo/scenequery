#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "FilePath.h"
#include "FileScanner.h"
#include "ActorThread.h"
#include "Scene.h"
#include "Actor.h"
#include "DatabaseThread.h"
#include <QtGlobal>
#include <QDebug>
#include <QVector>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    actorList = {};
    sceneList = {};
    this->currentDisplay = DISPLAY_ACTORS;
    qRegisterMetaType<int>("int");
    qRegisterMetaType<QString>("QString");
    qRegisterMetaType<QSharedPointer<Scene>>("QSharedPointer<Scene>");
    qRegisterMetaType<QSharedPointer<Actor>>("QSharedPointer<Actor>");
    qRegisterMetaType<QVector<QSharedPointer<Scene>>>("QVector<QSharedPointer<Scene>>");
    qRegisterMetaType<QVector<QSharedPointer<Actor>>>("QVector<QSharedPointer<Actor>>");
    qRegisterMetaType<ScenePtr>("ScenePtr");
    qRegisterMetaType<ActorPtr>("ActorPtr");
    qRegisterMetaType<SceneList>("SceneList");
    qRegisterMetaType<ActorList>("ActorList");
}

MainWindow::~MainWindow(){
    delete ui;
    actorList.clear();
    sceneList.clear();
}


void MainWindow::on_refreshScenes_clicked(){
    qDebug("Updating Scenes from Database");
    this->database = new DatabaseThread(sceneList, Database::OPERATION_FETCH);
    connect(database, SIGNAL(initProgress(int)),    this, SLOT(initProgress(int)));
    connect(database, SIGNAL(updateProgress(int)),  this, SLOT(updateProgress(int)));
    connect(database, SIGNAL(closeProgress()),      this, SLOT(finishProgress()));
    connect(database, SIGNAL(finished(SceneList)),  this, SLOT(receiveScenes(SceneList)));
    database->run();
}

void MainWindow::on_refreshActors_clicked()
{
    qDebug("Updating Scenes from Database");
    this->database = new DatabaseThread(actorList, Database::OPERATION_FETCH);
    connect(database, SIGNAL(initProgress(int)),    this, SLOT(initProgress(int)));
    connect(database, SIGNAL(updateProgress(int)),  this, SLOT(updateProgress(int)));
    connect(database, SIGNAL(closeProgress()),      this, SLOT(finishProgress()));

    connect(database, SIGNAL(finished(ActorList)),  this, SLOT(receiveScenes(ActorList)));
    database->run();
}

void MainWindow::on_actionScan_Directory_triggered(){
    QString path = QFileDialog::getExistingDirectory(this, "Pick Directory to Scan", "/Volumes");
    if (!path.isEmpty()){
        this->scanner = new FileScanner(path);
        connect(scanner, SIGNAL(finished(SceneList)), this, SLOT(receiveScenes(SceneList)));
        connect(scanner, SIGNAL(initProgress(int)),  this, SLOT(initProgress(int)));
        connect(scanner, SIGNAL(updateProgress(int)), this, SLOT(updateProgress(int)));
        connect(scanner, SIGNAL(closeProgress()), this, SLOT(finishProgress()));
        scanner->start();
    }
}

void MainWindow::on_radioButtonActors_clicked(){
    if (ui->radioButtonActors->isChecked()){
        this->currentDisplay = DISPLAY_ACTORS;
    } else {
        this->currentDisplay = DISPLAY_SCENES;
    }
    updateDisplayType();
}

void MainWindow::updateDisplayType(){
    if (currentDisplay == DISPLAY_SCENES){
        ui->actorView->hide();

    } else if (currentDisplay == DISPLAY_ACTORS){
        ui->actorView->show();
    }
}
void MainWindow::updateStatus(QString s){
    ui->statusBar->showMessage(s);
}

void MainWindow::initProgress(int count){
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(count);
    ui->progressBar->setMinimum(0);
}
void MainWindow::updateProgress(int value){
    ui->progressBar->setValue(value);
}
void MainWindow::finishProgress(){
    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->statusBar->showMessage("Finished");
}
/** \brief Slot to receive list of scenes. */
void MainWindow::receiveScenes(SceneList list){
    if (!this->sceneList.isEmpty()){
        foreach(ScenePtr s, list){
            if (!sceneList.contains(s)){
                sceneList.push_back(s);
            }
        }
    } else {
        this->sceneList = list;
    }
    qDebug("Scenes Updated");
}

void MainWindow::receiveActors(ActorList list){
    if (this->actorList.isEmpty()){
        actorList = list;
    } else {
        foreach(ActorPtr a, list){
            if (!actorList.contains(a)){
                actorList.push_back(a);
            }
        }
    }
    qDebug("Actor List updated from database");
}



