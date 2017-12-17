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
    DatabaseThread(List<class Scene>,    Database::Operation operation);
    DatabaseThread(List<class Actor>,    Database::Operation operation);
    ~DatabaseThread();
    void run();
    void setTable(Database::Table table);
    void setOperation(Database::Operation operation);
    void setList(List<class Scene> scenes);
    void setList(List<class Actor> actors);
private:
    List<class Actor> actorList;
    List<class Scene> sceneList;
    Database::Table table;
    Database::Operation operation;
    void updateSceneTable();
    void updateSceneList();
    void updateActorTable();
    void updateActorList();
    void insertActor(ActorPtr a);
    void insertScene(ScenePtr s);
    int index;
    QMutex mx;

signals:
    void finished(List<class Actor>);
    void finished(List<class Scene>);
    void initProgress(int count);
    void updateProgress(int currValue);
    void closeProgress();
};

#endif // DATABASETHREAD_H
