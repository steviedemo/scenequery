#ifndef INITIALIZATIONTHREAD_H
#define INITIALIZATIONTHREAD_H
#include <QThread>
#include "sql.h"
#include "definitions.h"
class InitializationThread : public QThread
{
    Q_OBJECT
public:
    InitializationThread();
    ~InitializationThread();
    void run();
private:
    SQL *sqlThread;
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
    void getActors(ActorList);
    void getScenes(SceneList);
    void stopSqlThread();
    void sendInitialLists(ActorList, SceneList);
};

#endif // INITIALIZATIONTHREAD_H
