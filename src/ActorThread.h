#ifndef ACTORTHREAD_H
#define ACTORTHREAD_H
#include <QThread>
#include <QMutex>
#include <QVector>
#include <QSharedPointer>
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
    QVector<QSharedPointer<class Actor>> readFromScenes();
    class DatabaseThread *database;
    QVector<QSharedPointer<class Actor>> actors;
    QVector<QSharedPointer<class Actor>> newList;
    QVector<QSharedPointer<class Scene>> scenes;
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
    void finished(QVector<QSharedPointer<class Actor>> );
    void updateStatus(QString);
};

#endif // ACTORTHREAD_H
