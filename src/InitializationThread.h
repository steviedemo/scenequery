#ifndef INITIALIZATIONTHREAD_H
#define INITIALIZATIONTHREAD_H
#include <QThread>
#include "SQL.h"
#include "definitions.h"
class InitializationThread : public QThread
{
    Q_OBJECT
public:
    InitializationThread();
    ~InitializationThread();
    void run();
private:
    bool waitingForScenes, waitingForActors;
    ActorList actors;
    SceneList scenes;
private slots:
    void receiveScenes(SceneList);
    void receiveActors(ActorList);
signals:
    void startProgress(QString, int);
    void updateProgress(int);
    void closeProgressDialog();
    void getActors();
    void getScenes();
    void stopSqlThread();
    void sendInitialLists(ActorList, SceneList);
};

#endif // INITIALIZATIONTHREAD_H
