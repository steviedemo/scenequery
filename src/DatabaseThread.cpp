#include "DatabaseThread.h"
#include "Actor.h"
#include "Scene.h"
#include "sql.h"
#include "definitions.h"
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
    SQL sql("db");
    connect(&sql, SIGNAL(startProgress(int)), this, SIGNAL(initProgress(int)));
    connect(&sql, SIGNAL(updateProgress(int)), this, SIGNAL(updateProgress(int)));
    connect(&sql, SIGNAL(closeProgress()),    this, SIGNAL(closeProgress()));
    if (table == Database::SCENE){
        if (operation == Database::OPERATION_UPDATE){
            emit updateStatus(QString("Updating Scene Database"));
            sql.updateDatabase(sceneList);
        } else {
            emit updateStatus("Getting Scenes from Database");
            sql.loadSceneList(sceneList);
            emit finished(sceneList);
        }
        emit updateStatus("Finished");
    } else {
        if (operation == Database::OPERATION_UPDATE){
            emit updateStatus("Updating Actor Database");
            sql.updateDatabase(actorList);
        } else {
            emit updateStatus("Getting Actors from Database");
            SQL::loadActorList(actorList);
            emit finished(actorList);
        }
        emit updateStatus("Finished");
    }
    disconnect(&sql, SIGNAL(startProgress(int)), this, SIGNAL(initProgress(int)));
    disconnect(&sql, SIGNAL(updateProgress(int)), this, SIGNAL(updateProgress(int)));
    disconnect(&sql, SIGNAL(closeProgress()),    this, SIGNAL(closeProgress()));
}

/** \brief Add an actor to the database */
void DatabaseThread::insertActor(QSharedPointer<Actor> a){
    if(!a.isNull()){
        SQL sql(a->getName());
        sql.actorSql(a, SQL_INSERT);
    }
}

/** \brief Add a Scene to the Database */
void DatabaseThread::insertScene(QSharedPointer<Scene> s){
    if (!s.isNull()){
        SQL sql(s->getTitle());
        sql.sceneSql(s, SQL_INSERT);
    }
}

bool DatabaseThread::setActor(ActorPtr a){
    QString name = a->getName();
    bool success = false;
    if (name.isEmpty()){
        qWarning("Not Creating Database Connection for actor object with empty name");
    } else {
        bool queryRan = false;
        SQL sql(a->getName());
        if(!sql.connect()){
            qWarning("Unable to create database connection with name '%s'", qPrintable(name));
        } else if(sql.hasActor(a, queryRan)){
            success = sql.actorSql(a, SQL_UPDATE);
        } else {
            success = sql.actorSql(a, SQL_INSERT);
        }
    }
    mx.lock();
    emit updateProgress(index++);
    mx.unlock();
    return success;
}

bool DatabaseThread::setScene(ScenePtr s){
    bool success = false, queryRan = false;
    SQL sql(s->getTitle());
    if (!sql.connect()){
        qWarning("Unable to connect to Scene Database");
    } else if (sql.hasScene(s, queryRan)){
        success = sql.sceneSql(s, SQL_UPDATE);
    } else {
        success = sql.sceneSql(s, SQL_INSERT);
    }
    mx.lock();
    emit updateProgress(index);
    mx.unlock();
    return success;
}
