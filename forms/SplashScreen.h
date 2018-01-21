#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QProgressBar>
#include <QCloseEvent>
#include <QShowEvent>
#include "SQL.h"
#include "definitions.h"
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
    ActorList actorList;
    SceneList sceneList;
signals:
    void startProgress(int);
    void startProgress(int ID, int max);
    void updateProgress(int);
    void updateProgress(int ID, int value);
    void closeProgress(void);
    void closeProgress(int ID);
    void done(ActorList);
    void done(SceneList);
};

class DisplayMaker : public QThread{
    Q_OBJECT
public:
    explicit DisplayMaker(ActorList &list, QObject *parent = 0);
    explicit DisplayMaker(SceneList &list, QObject *parent = 0);
    void run();
protected:
    void makeActorRow(ActorPtr a);
    void makeSceneRow(ScenePtr s);
private:
    QMutex mx;
    int index;
    ActorList actorList;
    SceneList sceneList;
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
    explicit SplashScreen(QWidget *parent = 0);
    ~SplashScreen();
public slots:
    void startProgress(int ID, int max);
    void updateProgress(int ID, int value);
    void finishProgress(int ID);
    void receiveScenes(SceneList);
    void receiveActors(ActorList);
    void receiveSceneDisplay(RowList);
    void receiveActorDisplay(RowList);
private slots:
    void stepComplete(int);
private:
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);
    Ui::SplashScreen *ui;
    QList<QProgressBar *>progressList;
    miniSQL *actorLoadThread, *sceneLoadThread;
    DisplayMaker *actorBuild, *sceneBuild;
    ActorList actors;
    SceneList scenes;
    bool scenesBuilt, actorsBuilt, scenesLoaded, actorsLoaded;
    QMutex mx;
    QVector<QList<QStandardItem *>>actorRows, sceneRows;
signals:
    void completed(int);
    void done(ActorList, SceneList, RowList, RowList);
};

#endif // SPLASHSCREEN_H
