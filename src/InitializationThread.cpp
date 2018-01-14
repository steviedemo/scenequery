#include "InitializationThread.h"
#include "Actor.h"
#include "SceneList.h"
InitializationThread::InitializationThread(){
    this->actors = {};
    this->scenes = {};
}
InitializationThread::~InitializationThread(){
}
void InitializationThread::run(){
    qDebug("Initialization Started");
    this->waitingForActors = true;
    this->waitingForScenes = true;

    emit getActors();
    while(waitingForActors){ sleep(1);  }

    qDebug("Got Initial Actors, Requesting Scenes");
    emit getScenes();
    while(waitingForScenes){ sleep(1);  }
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
