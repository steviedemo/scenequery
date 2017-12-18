#include "DatabaseThread.h"
#include "Actor.h"
#include "Scene.h"
#include <QtConcurrent>
#include <QFutureSynchronizer>

/** \brief Null Constructor */
DatabaseThread::DatabaseThread():index(0){
}
/** \brief Constructor without use of a previous list. */
DatabaseThread::DatabaseThread(Database::Table table, Database::Operation operation):
    table(table), operation(operation), index(0){
    this->operation = Database::OPERATION_NONE;
    this->actorList = {};
    this->sceneList = {};
}
/** \brief Perform series of database operations involving the actor list and actor table
 *  \param actors:      List of actor objects to add to or update database from
 *  \param operation:   Type of operation to perform
 */
DatabaseThread::DatabaseThread(ActorList actors, Database::Operation operation):
    actorList(actors), table(Database::ACTOR), operation(operation), index(0){
    this->sceneList = {};
}
/** \brief Perform series of database operations involving the scene list and scene table
 *  \param scenes:      List of scene objects to add to or update database from
 *  \param operation:   Type of operation to perform
 */
DatabaseThread::DatabaseThread(SceneList scenes, Database::Operation operation):
    sceneList(scenes), table(Database::SCENE), operation(operation), index(0){
    this->actorList = {};
}

DatabaseThread::~DatabaseThread(){
    sceneList.clear();
    actorList.clear();
}

void DatabaseThread::setList(ActorList actors)    {  this->actorList = actors;   }
void DatabaseThread::setList(SceneList scenes)    {  this->sceneList = scenes;   }
void DatabaseThread::setTable(Database::Table table){  this->table = table;    }
void DatabaseThread::setOperation(Database::Operation operation)    {   this->operation = operation;    }


void DatabaseThread::run(){
    index = 0;
    if (table == Database::SCENE){
        if (operation == Database::OPERATION_UPDATE){
            emit updateStatus(QString("Updating Scene Database"));
            updateSceneTable();
        } else {
            emit updateStatus("Getting Scenes from Database");
            updateSceneList();
            emit finished(sceneList);
        }
        emit updateStatus("Finished");
    } else {
        if (operation == Database::OPERATION_UPDATE){
            emit updateStatus("Updating Actor Database");
            updateActorTable();
        } else {
            emit updateStatus("Getting Actors from Database");
            updateActorList();
            emit finished(actorList);
        }
        emit updateStatus("Finished");
    }
}

void DatabaseThread::updateActorList(){
    SQL sql;
    sql.loadActorList(actorList);
}
void DatabaseThread::updateSceneList(){
    SQL sql;
    sql.loadSceneList(sceneList);
}
void DatabaseThread::updateActorTable(){
    if (actorList.size() > 0){
        qDebug("Updating Actor Database with %d Entries", actorList.size());
        QFutureSynchronizer<void> sync;
        emit initProgress(actorList.size());
        for (int i = 0; i < actorList.size(); ++i){
            sync.addFuture(QtConcurrent::run(this, &DatabaseThread::insertActor, actorList.at(i)));
        }
        sync.waitForFinished();
        emit closeProgress();
        qDebug("Actor Table Updated");
    }
}
void DatabaseThread::updateSceneTable(){
    if (sceneList.size() > 0){
        qDebug("Updating Scene Table with %d Entries", sceneList.size());
        QFutureSynchronizer<void> sync;
        emit initProgress(sceneList.size());
        for (int i = 0; i < sceneList.size(); ++i){
            sync.addFuture(QtConcurrent::run(this, &DatabaseThread::insertScene, sceneList.at(i)));
        }
        sync.waitForFinished();
        emit closeProgress();
        qDebug("Scene Table Updated");
    }
}

bool DatabaseThread::setActor(ActorPtr a){
    QString name = a->getName();
    bool success = false;
    if (name.isEmpty()){
        qWarning("Not Creating Database Connection for actor object with empty name");
    } else {
        SQL sql(a->getName());
        if(!sql.connect()){
            qWarning("Unable to create database connection with name '%s'", qPrintable(name));
        } else if(sql.hasActor(a)){
            success = sql.actorSql(a, Database::UPDATE);
        } else {
            success = sql.actorSql(a, Database::INSERT);
        }
    }
    mx.lock();
    emit updateProgress(index++);
    mx.unlock();
    return success;
}

bool DatabaseThread::setScene(ScenePtr s){
    bool success = false;
    SQL sql(s->getTitle());
    if (!sql.connect()){
        qWarning("Unable to connect to Scene Database");
    } else if (sql.hasScene(s)){
        success = sql.sceneSql(s, Database::UPDATE);
    } else {
        success = sql.sceneSql(s, Database::INSERT);
    }
    mx.lock();
    emit updateProgress(index);
    mx.unlock();
    return success;
}
