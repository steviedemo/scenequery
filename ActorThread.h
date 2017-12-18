#ifndef ACTORTHREAD_H
#define ACTORTHREAD_H
#include "definitions.h"
#include "DatabaseThread.h"
#include "Actor.h"
#include "Scene.h"
#include <QThread>
#include <QMutex>
class ActorThread : public QThread
{
public:
    ActorThread();
    ActorThread(ActorList);
    ActorThread(SceneList);
    ActorThread(ActorList, SceneList);
    ~ActorThread();
    void run();
private:
    void readFromDatabase();
    ActorList readFromScenes();
    DatabaseThread *database;
    ActorList actors;
    ActorList newList;
    SceneList scenes;
    QStringList names;
    int index;
    QMutex mx;
    bool dbThreadFinished;
    void updateBio(ActorPtr a);
private slots:
    void receiveActors(ActorList);
signals:
    void initProgress(int);
    void updateProgress(int);
    void closeProgress(void);
    void finished(ActorList);
    void updateStatus(QString);
};

#endif // ACTORTHREAD_H
