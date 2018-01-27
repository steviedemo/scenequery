#include "SplashScreen.h"
#include "config.h"
#include "ui_SplashScreen.h"
#include "Actor.h"
#include "Scene.h"
#include <QFutureSynchronizer>
#include <QtConcurrent>
#include <QtConcurrentRun>
SplashScreen::SplashScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SplashScreen),
    scenesBuilt(false), actorsBuilt(false), scenesLoaded(false), actorsLoaded(false)
{
    ui->setupUi(this);
    checkForDataDirectories();
    progressList << ui->pb_loadActors << ui->pb_loadScenes << ui->pb_buildActors << ui->pb_buildScenes;
    foreach(QProgressBar *pb, progressList){
        pb->setMinimum(0);
        pb->setValue(0);
    }
    this->scenes = {};
    this->actors = {};
    this->actorBuildThreadDone = false;
    this->sceneBuildThreadDone = false;
}

void SplashScreen::showEvent(QShowEvent *){
    this->sceneLoadThread = new miniSQL("scenes");
    this->actorLoadThread = new miniSQL("actors");
    connect(sceneLoadThread, SIGNAL(closeProgress(int)),      this, SLOT(finishProgress(int)));
    connect(actorLoadThread, SIGNAL(closeProgress(int)),      this, SLOT(finishProgress(int)));
    connect(sceneLoadThread, SIGNAL(startProgress(int,int)),  this, SLOT(startProgress(int,int)));
    connect(actorLoadThread, SIGNAL(startProgress(int,int)),  this, SLOT(startProgress(int,int)));
    connect(sceneLoadThread, SIGNAL(updateProgress(int,int)), this, SLOT(updateProgress(int,int)));
    connect(actorLoadThread, SIGNAL(updateProgress(int,int)), this, SLOT(updateProgress(int,int)));
    connect(sceneLoadThread, SIGNAL(done(SceneList)),         this, SLOT(receiveScenes(SceneList)));
    connect(actorLoadThread, SIGNAL(done(ActorList)),         this, SLOT(receiveActors(ActorList)));
    connect(this, SIGNAL(completed(int)), this, SLOT(stepComplete(int)));
    sceneLoadThread->start();
    actorLoadThread->start();
}
void SplashScreen::closeEvent(QCloseEvent *event){
    event->accept();
}

SplashScreen::~SplashScreen(){
    delete ui;
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
void SplashScreen::actorBuildThreadFinished(){
    this->actorBuildThreadDone = true;
    qDebug("Actor Build Thread finished");
}
void SplashScreen::sceneBuildThreadFinished(){
    this->sceneBuildThreadDone = true;
    qDebug("Scene Build Thread Finished");
}

void SplashScreen::receiveActors(ActorList list){
    this->actors = list;
    qDebug("Starting to build %d actor display items", list.size());
    emit completed(LOAD_ACTOR_PROGRESS);
    QMutexLocker ml(&mx);
    this->actorBuild = new DisplayMaker(actors, this);
    connect(actorBuild, SIGNAL(done(RowList)),      this, SLOT(receiveActorDisplay(RowList)));
    connect(actorBuild, SIGNAL(startRun(int,int)),  this, SLOT(startProgress(int,int)));
    connect(actorBuild, SIGNAL(stopRun(int)),       this, SLOT(finishProgress(int)));
    connect(actorBuild, SIGNAL(update(int,int)),    this, SLOT(updateProgress(int,int)));
    connect(actorBuild, SIGNAL(finished()),         this, SLOT(sceneBuildThreadFinished()));
    actorBuild->start();
}

void SplashScreen::receiveScenes(SceneList list){
    this->scenes = list;
    qDebug("Starting to build %d scene display items", list.size());
    emit completed(LOAD_SCENE_PROGRESS);
    QMutexLocker ml(&mx);
    this->sceneBuild = new DisplayMaker(scenes, this);
    connect(sceneBuild, SIGNAL(done(RowList)),      this, SLOT(receiveSceneDisplay(RowList)));
    connect(sceneBuild, SIGNAL(startRun(int,int)),  this, SLOT(startProgress(int,int)));
    connect(sceneBuild, SIGNAL(stopRun(int)),       this, SLOT(finishProgress(int)));
    connect(sceneBuild, SIGNAL(update(int,int)),    this, SLOT(updateProgress(int,int)));
    connect(sceneBuild, SIGNAL(finished()),         this, SLOT(actorBuildThreadFinished()));
    sceneBuild->start();
}

void SplashScreen::stepComplete(int progress){
    QMutexLocker ml(&mx);
    if (progress == LOAD_ACTOR_PROGRESS){
        this->actorsLoaded = true;
        this->actorLoadThread->deleteLater();
    } else if (progress == LOAD_SCENE_PROGRESS){
        this->scenesLoaded = true;
        this->sceneLoadThread->deleteLater();
    } else if (progress == BUILD_ACTOR_PROGRESS){
        this->actorsBuilt = true;
        if (actorBuild){
            this->actorBuild->quit();
            this->actorBuild->wait();
            this->actorBuild->deleteLater();
        }
    } else if (progress == BUILD_SCENE_PROGRESS){
        this->scenesBuilt = true;
        if (sceneBuild){
            this->sceneBuild->quit();
            this->sceneBuild->wait();
            this->sceneBuild->deleteLater();
        }
    }
    qDebug("\nIndex %d Finished\n", progress);
    progressList[progress]->setValue(progressList.at(progress)->maximum());
    if (actorsLoaded && scenesLoaded && actorsBuilt && scenesBuilt){
        ml.unlock();
        qDebug("Initialization Finished");
        emit done(actors, scenes, actorRows, sceneRows);
    }
}

void SplashScreen::receiveActorDisplay(QVector<QList<QStandardItem *> > rows){
    this->actorRows = rows;
    qDebug("Got %d actor rows", rows.size());
    //this->actorBuild->quit();
    emit completed(BUILD_ACTOR_PROGRESS);
}
void SplashScreen::receiveSceneDisplay(QVector<QList<QStandardItem *> > rows){
    this->sceneRows = rows;
    qDebug("Got %d scene Rows", rows.size());
    //this->sceneBuild->quit();
    emit completed(BUILD_SCENE_PROGRESS);
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
            sync.addFuture(QtConcurrent::run(this, &DisplayMaker::makeSceneRow, s));
        } else {
            ActorPtr a = actorList.at(i);
            sync.addFuture(QtConcurrent::run(this, &DisplayMaker::makeActorRow, a));
        }
    }
    sync.waitForFinished();
    emit done(rows);
}

void DisplayMaker::makeActorRow(ActorPtr a){
    QList<QStandardItem *> row = a->buildQStandardItem();
    QMutexLocker ml(&mx);
    this->rows << row;
    if (++index % 50 == 0){
        emit update(BUILD_ACTOR_PROGRESS, index);
    }
}
void DisplayMaker::makeSceneRow(ScenePtr s){
    QList<QStandardItem*>row = s->buildQStandardItem();
    QMutexLocker ml(&mx);
    this->rows << row;
    if (++index %100 == 0){
        emit update(BUILD_SCENE_PROGRESS, index);
    }
}



void miniSQL::run(){
    const int task = ((currentTable==SCENE) ? LOAD_SCENE_PROGRESS : LOAD_ACTOR_PROGRESS);
    QString statement = QString("SELECT * FROM %1").arg((currentTable==SCENE) ? "scenes" : "actors");
    sqlConnection connection(statement);
    if (!connection.execute()){
        qWarning("Error Loading items from database!");
    } else {
        pqxx::result r = connection.getResult();
        if (r.size() == 0){
            qWarning("No Items in table");
            return;
        }
        emit startProgress(task, r.size());
        int idx = 0;
        for(pqxx::result::const_iterator i = r.begin(); i != r.end(); ++i){
            if (currentTable == SCENE){
                ScenePtr s = ScenePtr(new Scene(i));
                sceneList.push_back(s);
            } else {
                ActorPtr a = ActorPtr(new Actor(i));
                actorList.push_back(a);
            }
            emit updateProgress(task, idx++);
        }
        int items = 0;
        if (currentTable == SCENE){
            items = sceneList.size();
        } else {
            items = actorList.size();
        }
        qDebug("miniSQL Thread loaded %d items with '%s'", items, qPrintable(statement));
    }
    emit closeProgress(task);
    if (currentTable == SCENE){
        emit done(sceneList);
    } else {
        emit done(actorList);
    }
}

