#ifndef DATABASETHREAD_H
#define DATABASETHREAD_H

#include <QString>
#include <QThread>
#include <QMutex>
#include "definitions.h"
#include <QSharedPointer>
#include <QVector>

class DatabaseThread : public QThread
{
    Q_OBJECT
public:
    DatabaseThread();
    DatabaseThread(Database::Table table,Database::Operation operation);
    DatabaseThread(SceneList,    Database::Operation operation);
    DatabaseThread(ActorList,    Database::Operation operation);
    ~DatabaseThread();
    void run();
    void setTable(Database::Table table);
    void setOperation(Database::Operation operation);
    void setList(QVector<QSharedPointer<class Scene>> scenes);
    void setList(QVector<QSharedPointer<class Actor>> actors);
private:
    ActorList actorList;
    SceneList sceneList;
    Database::Table table;
    Database::Operation operation;
    void updateSceneList();
    void updateActorList();
    void insertActor(QSharedPointer<class Actor> a);
    void insertScene(QSharedPointer<class Scene> s);
    bool setActor(QSharedPointer<class Actor> a);
    bool setScene(QSharedPointer<class Scene> s);
    int index;
    QMutex mx;

signals:
    void finished(ActorList);
    void finished(SceneList);
    void initProgress(int count);
    void updateProgress(int currValue);
    void closeProgress();
    void updateStatus(QString);
};

#endif // DATABASETHREAD_H
