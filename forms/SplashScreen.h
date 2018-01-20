#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QProgressBar>
#include "SQL.h"
#include "definitions.h"
namespace Ui {
class SplashScreen;
}
class DisplayMaker : public QThread{
    Q_OBJECT
public:
    explicit DisplayMaker(ActorList &list, QObject *parent = 0);
    explicit DisplayMaker(SceneList &list, QObject *parent = 0);
    void run();
protected:
    void makeRow(ActorPtr a);
    void makeRow(ScenePtr s);
private:
    QMutex mx;
    int index;
    ActorList actorList;
    SceneList sceneList;
    QString listType;
    QVector<QList<QStandardItem *>> rows;
signals:
    void done(QVector<QList<QStandardItem *>>);
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
    void receiveSceneDisplay(QVector<QList<QStandardItem *>> rows);
    void receiveActorDisplay(QVector<QList<QStandardItem *>> rows);
private slots:
private:
    Ui::SplashScreen *ui;
    QList<QProgressBar *>progressList;
    miniSQL *actorLoadThread, *sceneLoadThread;
    DisplayMaker *actorBuild, *sceneBuild;
    ActorList actors;
    SceneList scenes;
    QVector<QList<QStandardItem *>>actorRows, sceneRows;
signals:
    void done(ActorList, SceneList, QVector<QList<QStandardItem *>>, QVector<QList<QStandardItem *>>);
};

#endif // SPLASHSCREEN_H
