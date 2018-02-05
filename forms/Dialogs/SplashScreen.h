#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H
#include <QThread>
#include <QWidget>
#include <QProgressBar>
#include <QCloseEvent>
#include <QShowEvent>
#include "DataManager.h"
#include "definitions.h"
#include "SQL.h"
namespace Ui {
class SplashScreen;
}

class miniSQL : public QThread {
    Q_OBJECT
public :
    explicit miniSQL(QString table){    currentTable = ((table=="scenes") ? SCENE : ACTOR); }
    ~miniSQL() {}
    void run();
private:
    enum Table{ ACTOR, SCENE };
    Table currentTable;
    ActorMap actorMap;
    SceneMap sceneMap;
signals:
    void startProgress(int);
    void startProgress(int ID, int max);
    void updateProgress(int);
    void updateProgress(int ID, int value);
    void closeProgress(void);
    void closeProgress(int ID);
    void done(ActorMap);
    void done(SceneMap);
};

class DisplayMaker : public QThread{
    Q_OBJECT
public:
    explicit DisplayMaker(ActorMap &list, QObject *parent = 0): QThread(parent), actorMap(list), listType("actors"){}
    explicit DisplayMaker(SceneMap &list, QObject *parent = 0): QThread(parent), sceneMap(list), listType("scenes"){}
    ~DisplayMaker(){}
    void run();
protected:
    void makeActorRow(ActorPtr a);
    void makeSceneRow(ScenePtr s);
private:
    QMutex mx;
    int index;
    ActorMap actorMap;
    SceneMap sceneMap;
    QString listType;
    RowList rows;
signals:
    void completed(int);
    void done(RowList);
    void startRun(int id, int max);
    void update(int id, int value);
    void stopRun(int id);
};

class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(QSharedPointer<DataManager> vault, QWidget *parent = 0);
    ~SplashScreen();
public slots:
    void startProgress(int ID, int max);
    void updateProgress(int ID, int value);
    void finishProgress(int ID);
private slots:
    void stepComplete(int);
    void receiveScenes(SceneMap);
    void receiveActors(ActorMap);
    void receiveSceneDisplay(RowList);
    void receiveActorDisplay(RowList);
    void sceneRowsLoaded();
    void actorRowsLoaded();
private:
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);
    Ui::SplashScreen *ui;
    QList<QProgressBar *>progressList;
    miniSQL *actorLoadThread, *sceneLoadThread;
    DisplayMaker *actorBuild, *sceneBuild;
    ActorList actors;
    SceneList scenes;
    bool actorBuildThreadDone, sceneBuildThreadDone;
    bool scenesBuilt, actorsBuilt, scenesLoaded, actorsLoaded;
    QMutex mx;
    RowList actorRows, sceneRows;
    ActorMap actorMap;
    SceneMap sceneMap;
    QSharedPointer<DataManager>vault;
signals:
    void completed(int);
    void sendActorRows  (RowList);
    void sendSceneRows  (RowList);
    void sendActors     (ActorMap);
    void sendScenes     (SceneMap);
    void done           (void);
};

#endif // SPLASHSCREEN_H
