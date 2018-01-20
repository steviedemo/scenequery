#include "SplashScreen.h"
#include "ui_SplashScreen.h"
#include "Actor.h"
#include "Scene.h"
#include <QtConcurrent>
#include <QtConcurrentRun>
SplashScreen::SplashScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SplashScreen)
{
    ui->setupUi(this);
    progressList << ui->pb_loadActors << ui->pb_loadScenes << ui->pb_buildActors << ui->pb_buildScenes;
    foreach(QProgressBar *pb, progressList){
        pb->setMinimum(0);
        pb->setValue(0);
    }
    this->sceneLoadThread = new miniSQL("scenes");
    this->actorLoadThread = new miniSQL("actors");
    this->scenes = {};
    this->actors = {};
    connect(sceneLoadThread, SIGNAL(closeProgress(int)),      this, SLOT(finishProgress(int)));
    connect(actorLoadThread, SIGNAL(closeProgress(int)),      this, SLOT(finishProgress(int)));
    connect(sceneLoadThread, SIGNAL(startProgress(int,int)),  this, SLOT(startProgress(int,int)));
    connect(actorLoadThread, SIGNAL(startProgress(int,int)),  this, SLOT(startProgress(int,int)));
    connect(sceneLoadThread, SIGNAL(updateProgress(int,int)), this, SLOT(updateProgress(int,int)));
    connect(actorLoadThread, SIGNAL(updateProgress(int,int)), this, SLOT(updateProgress(int,int)));
    connect(sceneLoadThread, SIGNAL(done(SceneList)),         this, SLOT(receiveScenes(SceneList)));
    connect(actorLoadThread, SIGNAL(done(ActorList)),         this, SLOT(receiveActors(ActorList)));
    sceneLoadThread->start();
    actorLoadThread->start();
}

SplashScreen::~SplashScreen(){
    delete ui;
    delete actorBuild;
    delete sceneBuild;
}

void SplashScreen::startProgress(int ID, int max){
    if (ID < progressList.size() && ID > -1){
        progressList.at(ID)->setMaximum(max);
    }
}
void SplashScreen::updateProgress(int ID, int value){
    if (ID < progressList.size() && ID > -1){
        progressList.at(ID)->setValue(value);
    }
}
void SplashScreen::finishProgress(int ID){
    if (ID < progressList.size() && ID > -1){
        progressList.at(ID)->setValue(progressList.at(ID)->maximum());
    }
}
void SplashScreen::receiveActors(ActorList list){
    this->actors = list;
    qDebug("Starting to build %d actor display items", list.size());
    this->actorLoadThread->deleteLater();
    this->actorBuild = new DisplayMaker(actors, this);
    connect(actorBuild, SIGNAL(done(QVector<QList<QStandardItem*> >)), this, SLOT(receiveSceneDisplay(QVector<QList<QStandardItem*> >)));
    connect(actorBuild, SIGNAL(startRun(int,int)), this, SLOT(startProgress(int,int)));
    connect(actorBuild, SIGNAL(stopRun(int)), this, SLOT(finishProgress(int)));
    connect(actorBuild, SIGNAL(update(int,int)), this, SLOT(updateProgress(int,int)));
    actorBuild->start();
}

void SplashScreen::receiveScenes(SceneList list){
    this->scenes = list;
    qDebug("Starting to build %d scene display items", list.size());
    this->sceneLoadThread->deleteLater();
    this->sceneBuild = new DisplayMaker(scenes, this);
    connect(sceneBuild, SIGNAL(done(QVector<QList<QStandardItem*> >)), this, SLOT(receiveSceneDisplay(QVector<QList<QStandardItem*> >)));
    connect(sceneBuild, SIGNAL(startRun(int,int)), this, SLOT(startProgress(int,int)));
    connect(sceneBuild, SIGNAL(stopRun(int)), this, SLOT(finishProgress(int)));
    connect(sceneBuild, SIGNAL(update(int,int)), this, SLOT(updateProgress(int,int)));
    sceneBuild->start();
}

void SplashScreen::receiveActorDisplay(QVector<QList<QStandardItem *> > rows){
    this->actorRows = rows;
    qDebug("Got %d actor rows", rows.size());
    this->actorBuild->deleteLater();
    if (!sceneRows.isEmpty()){
        qDebug("Init finished!");
        emit done(actors, scenes, actorRows, sceneRows);
    }
}
void SplashScreen::receiveSceneDisplay(QVector<QList<QStandardItem *> > rows){
    this->sceneRows = rows;
    qDebug("Got %d scene Rows", rows.size());
    this->sceneBuild->deleteLater();
}

DisplayMaker::DisplayMaker(ActorList &list, QObject *parent):
    QThread(parent), actorList(list), listType("actors"){
}
DisplayMaker::DisplayMaker(SceneList &list, QObject *parent):
    QThread(parent), sceneList(list), listType("scenes"){
}

void DisplayMaker::run(){
    this->index = 0;
    int total = 0, id = -1;
    if (listType == "scenes"){
        total = sceneList.size();
        id = BUILD_SCENE_PROGRESS;
    } else {
        total = actorList.size();
        id = BUILD_ACTOR_PROGRESS;
    }
    emit startRun(id, total);
    QFutureSynchronizer<void> sync;
    for( int i = 0; i < total; ++i){
        if (listType == "scenes"){
            ScenePtr s = sceneList.at(i);
            sync.addFuture(QtConcurrent::run(this, &DisplayMaker::makeRow, s));
        } else {
            ActorPtr a = actorList.at(i);
            sync.addFuture(QtConcurrent::run(this, &DisplayMaker::makeRow, a));
        }
    }
    sync.waitForFinished();
    emit stopRun(id);
    emit done(rows);
}

void DisplayMaker::makeRow(ActorPtr a){
    QList<QStandardItem *> row = a->buildQStandardItem();
    QMutexLocker ml(&mx);
    this->rows << row;
    if (++index % 50 == 0){
        emit update(BUILD_ACTOR_PROGRESS, index);
    }
}
void DisplayMaker::makeRow(ScenePtr s){
    QList<QStandardItem*>row = s->buildQStandardItem();
    QMutexLocker ml(&mx);
    this->rows << row;
    if (++index %100 == 0){
        emit update(BUILD_SCENE_PROGRESS, index);
    }
}
