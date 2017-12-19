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
    Q_OBJECT
public:
    ActorThread();
    ActorThread(QVector<QSharedPointer<class Actor>>);
    ActorThread(QVector<QSharedPointer<class Scene>>);
    ActorThread(QVector<QSharedPointer<class Actor>>, QVector<QSharedPointer<class Scene>>);
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
    void updateBio(QSharedPointer<class Actor> a);
private slots:
    void receiveActors(QVector<QSharedPointer<class Actor>>);
signals:
    void initProgress(int);
    void updateProgress(int);
    void closeProgress(void);
    void finished(ActorList);
    void updateStatus(QString);
};

#endif // ACTORTHREAD_H
