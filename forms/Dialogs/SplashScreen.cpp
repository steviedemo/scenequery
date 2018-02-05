#include "SplashScreen.h"
#include "config.h"
#include "ui_SplashScreen.h"
#include "Actor.h"
#include "Scene.h"
#include <QFutureSynchronizer>
#include <QtConcurrent>
#include <QtConcurrentRun>
SplashScreen::SplashScreen(QSharedPointer<DataManager> vault, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SplashScreen),
    scenesBuilt(false), actorsBuilt(false), scenesLoaded(false), actorsLoaded(false), vault(vault)
{
    ui->setupUi(this);
    checkForDataDirectories();
    progressList << ui->pb_loadActors << ui->pb_loadScenes << ui->pb_buildActors << ui->pb_buildScenes;
    foreach(QProgressBar *pb, progressList){
        pb->setMinimum(0);
        pb->setValue(0);
    }
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
    connect(sceneLoadThread, SIGNAL(done(SceneMap)),          this, SLOT(receiveScenes(SceneMap)));
    connect(actorLoadThread, SIGNAL(done(ActorMap)),          this, SLOT(receiveActors(ActorMap)));
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

void SplashScreen::receiveActors(ActorMap list){
    qDebug("Adding %d items to Actor Map", list.size());
    vault->setMap(list);
    qDebug("Starting to build %d actor display items", list.size());
    this->actorBuild = new DisplayMaker(list);
    connect(actorBuild, SIGNAL(done(RowList)),      this, SLOT(receiveActorDisplay(RowList)));
    connect(actorBuild, SIGNAL(startRun(int,int)),  this, SLOT(startProgress(int,int)));
    connect(actorBuild, SIGNAL(stopRun(int)),       this, SLOT(finishProgress(int)));
    connect(actorBuild, SIGNAL(update(int,int)),    this, SLOT(updateProgress(int,int)));
    actorBuild->start();

    emit completed(LOAD_ACTOR_PROGRESS);
}

void SplashScreen::receiveScenes(SceneMap list){
    qDebug("Adding %d items to scene map", list.size());
    vault->setMap(list);
    qDebug("Starting to build %d scene display items", list.size());
    this->sceneBuild = new DisplayMaker(list);
    connect(sceneBuild, SIGNAL(done(RowList)),      this, SLOT(receiveSceneDisplay(RowList)));
    connect(sceneBuild, SIGNAL(startRun(int,int)),  this, SLOT(startProgress(int,int)));
    connect(sceneBuild, SIGNAL(stopRun(int)),       this, SLOT(finishProgress(int)));
    connect(sceneBuild, SIGNAL(update(int,int)),    this, SLOT(updateProgress(int,int)));
    sceneBuild->start();
    emit completed(LOAD_SCENE_PROGRESS);
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
        this->actorBuild->deleteLater();
    } else if (progress == BUILD_SCENE_PROGRESS){
        this->scenesBuilt = true;
        this->sceneBuild->deleteLater();
    }
    qDebug("\nIndex %d Finished\n", progress);
    progressList[progress]->setRange(0, 100);
    progressList[progress]->setValue(100);
    if (actorsLoaded && scenesLoaded && actorsBuilt && scenesBuilt){
        ml.unlock();
        qDebug("Initialization Finished");
        emit done();
    }
}

void SplashScreen::receiveActorDisplay(RowList rows){
    this->actorRows = rows;
    qDebug("Got %d actor rows", rows.size());
    emit sendActorRows(rows);
}
void SplashScreen::actorRowsLoaded(){
    qDebug("Actor Rows Finished Loading into the Table View");
    emit completed(BUILD_ACTOR_PROGRESS);
}
void SplashScreen::receiveSceneDisplay(RowList rows){
    this->sceneRows = rows;
    qDebug("Got %d scene Rows", rows.size());
    emit sendSceneRows(sceneRows);
}
void SplashScreen::sceneRowsLoaded(){
    qDebug("Scene Rows Finished Loading into the Table View");
    emit completed(BUILD_SCENE_PROGRESS);
}

void DisplayMaker::run(){
    this->index = 0;
    int id = -1;
    QFutureSynchronizer<void> sync;
    if (listType == "actors"){
        id = BUILD_ACTOR_PROGRESS;
        emit startRun(BUILD_ACTOR_PROGRESS, actorMap.size());
        QHashIterator<QString, ActorPtr> it(actorMap);
        while(it.hasNext()){
            it.next();
            sync.addFuture(QtConcurrent::run(this, &DisplayMaker::makeActorRow, it.value()));
        }
    } else {
        id = BUILD_SCENE_PROGRESS;
        emit startRun(BUILD_SCENE_PROGRESS, sceneMap.size());
        QHashIterator<int, ScenePtr> it(sceneMap);
        while(it.hasNext()){
            it.next();
            sync.addFuture(QtConcurrent::run(this, &DisplayMaker::makeSceneRow, it.value()));
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
        int index = 0;
        emit startProgress(task, r.size());
        for(pqxx::result::const_iterator i = r.begin(); i != r.end(); ++i){
            if (currentTable == SCENE){
                ScenePtr s = ScenePtr(new Scene(i));
                sceneMap.insert(s->getID(), s);
            } else {
                ActorPtr a = ActorPtr(new Actor(i));
                actorMap.insert(a->getName(), a);
            }
            emit updateProgress(task, index++);
        }
        int items = ((currentTable == SCENE) ? sceneMap.size() : actorMap.size());
        qDebug("miniSQL Thread loaded %d items with '%s'", items, qPrintable(statement));
        emit closeProgress(task);
    }
    if (currentTable == SCENE){
        emit done(sceneMap);
    } else {
        emit done(actorMap);
    }
}

