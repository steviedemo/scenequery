#ifndef DATABASETHREAD_H
#define DATABASETHREAD_H

#include <QString>
#include <QThread>
#include <QMutex>
#include "definitions.h"
#include "sql.h"


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
    void setList(SceneList scenes);
    void setList(ActorList actors);
private:
    ActorList actorList;
    SceneList sceneList;
    Database::Table table;
    Database::Operation operation;
    void updateSceneTable();
    void updateSceneList();
    void updateActorTable();
    void updateActorList();
    void insertActor(ActorPtr a);
    void insertScene(ScenePtr s);
    bool setActor(ActorPtr a);
    bool setScene(ScenePtr s);
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
