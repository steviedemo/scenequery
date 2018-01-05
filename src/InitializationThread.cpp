#include "InitializationThread.h"
#include "Actor.h"
#include "SceneList.h"
InitializationThread::InitializationThread(){
    this->actors = {};
    this->scenes = {};
    this->sqlThread = new SQL();
    connect(sqlThread, SIGNAL(startProgress(QString, int)), this, SIGNAL(startProgress(QString, int)));
    connect(sqlThread, SIGNAL(updateProgress(int)),         this, SIGNAL(updateProgress(int)));
    connect(sqlThread, SIGNAL(closeProgress(QString)),      this, SIGNAL(closeProgressDialog()));
    connect(sqlThread, SIGNAL(sendResult(ActorList)),       this, SLOT(receiveActors(ActorList)));
    connect(sqlThread, SIGNAL(sendResult(SceneList)),       this, SLOT(receiveScenes(SceneList)));
    connect(this,      SIGNAL(getActors(ActorList)),   sqlThread, SLOT(load(ActorList)));
    connect(this,      SIGNAL(getScenes(SceneList)),   sqlThread, SLOT(load(SceneList)));
    connect(this,      SIGNAL(stopSqlThread()),        sqlThread, SLOT(stopThread()));
}
InitializationThread::~InitializationThread(){
    delete sqlThread;
}
void InitializationThread::run(){
    qDebug("Initialization Started");
    this->waitingForActors = true;
    this->waitingForScenes = true;

    emit getActors(actors);
    while(waitingForActors){ sleep(1);  }

    qDebug("Got Initial Actors, Requesting Scenes");
    emit getScenes(scenes);
    while(waitingForScenes){ sleep(1);  }
    emit stopSqlThread();
    qDebug("Got Initial Scenes. Initialization Complete");
    emit sendInitialLists(actors, scenes);
}

void InitializationThread::receiveActors(ActorList list){
    this->actors = list;
    this->waitingForActors = false;
}

void InitializationThread::receiveScenes(SceneList list){
    qDebug("\n\tInitialization Thread Received %d Scenes\n", list.size());
    this->scenes = list;
    this->waitingForScenes = false;
}
