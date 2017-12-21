#include "ActorThread.h"
#include "definitions.h"
#include "DatabaseThread.h"
#include "Scene.h"
#include "Actor.h"
#include <QFutureSynchronizer>
#include <qtconcurrentrun.h>
ActorThread::ActorThread(){
    this->scenes = {};
    this->actors = {};
}
ActorThread::ActorThread(ActorList list):actors(list){
    this->scenes = {};
    foreach(ActorPtr a, list){
        names << a->getName();
    }
}
ActorThread::ActorThread(SceneList list):scenes(list){
    this->actors = {};
}
ActorThread::ActorThread(ActorList a, SceneList s):
    actors(a), scenes(s){
}
ActorThread::~ActorThread(){
    actors.clear();
    scenes.clear();
}

void ActorThread::run(){
    readFromDatabase();
    if (!scenes.isEmpty()){
        readFromScenes();
    }
    while(!dbThreadFinished){sleep(1);}
    emit finished(newList);
}

void ActorThread::readFromDatabase(){
    emit updateStatus("Updating Actors from Database");
    database = new DatabaseThread(actors, Database::OPERATION_FETCH);
    connect(database, SIGNAL(initProgress(int)), this, SIGNAL(initProgress(int)));
    connect(database, SIGNAL(updateProgress(int)), this, SIGNAL(updateProgress(int)));
    connect(database, SIGNAL(closeProgress()), this, SIGNAL(closeProgress()));
    connect(database, SIGNAL(finished(QVector<QSharedPointer<Actor>>)), this, SLOT(receiveActors(QVector<QSharedPointer<Actor>>)));
    database->start();
}

void ActorThread::receiveActors(ActorList list){
    dbThreadFinished = false;
    qDebug("Updating Actors from database");
    foreach(ActorPtr a, list){
        if (!actors.contains(a)){
            mx.lock();
            newList.push_back(a);
            names << a->getName();
            mx.unlock();
        }
    }
    dbThreadFinished = true;
}

void ActorThread::updateBio(ActorPtr a){
    a->updateBio();
    mx.lock();
    emit updateProgress(index++);
    this->newList.push_back(a);
    mx.unlock();
}

ActorList ActorThread::readFromScenes(){
    index = 0;
    QStringList newNames;
    qDebug("Scanning %d Scenes for new names.", scenes.size());
    foreach(ScenePtr s, scenes){
        QStringList cast = s->getActors();
        foreach(QString name, cast){
            if (!names.contains(name)){
                newNames << name;
                names << name;
            }
        }
    }
    // Make Actor objects out of list of new names.
    qDebug("%d Names found in scenes that weren't in the database.", newNames.size());
    emit updateStatus("Updating Actor bios from the Web");
    int total = newNames.size();
    emit initProgress(total);
    QFutureSynchronizer<void> sync;
    foreach (QString name, newNames){
        ActorPtr a = QSharedPointer<Actor>(new Actor(name));
        sync.addFuture(QtConcurrent::run(this, &ActorThread::updateBio, a));
    }
    sync.waitForFinished();
    emit closeProgress();
    return newList;
}
