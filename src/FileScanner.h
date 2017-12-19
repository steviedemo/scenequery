#ifndef FILESCANNER_H
#define FILESCANNER_H
#include "FilePath.h"
#include "definitions.h"
// Qt Stuff
#include <QObject>
#include <QStandardItem>
#include <QString>
#include <QThread>
// Data Parsing
#include <QDate>
#include <QFile>
#include <QFileInfo>
// Collection Structures
#include <QVector>
#include <QMap>
#include <QMutex>

class FileScanner : public QThread
{
    Q_OBJECT
public:
    FileScanner(QString path="");
    ~FileScanner();
    bool setPath(QString);
    void run();
public slots:
    void receiveFileVector(QVector<FilePath>);

private:
    void recursiveScan(QString);
    void addScene(FilePath);
    QFileInfoList scan();
    QFileInfoList recursiveScan(QFileInfo);
    void initializeActors(ActorList);
    ActorList parseActorList(SceneList);
    SceneList parseSceneList(QFileInfoList);
    QString scanDir;
    QVector<FilePath> files;
    ActorList actors;
    SceneList scenes;
    int index;
    QMutex mx;
signals:
    void finished(SceneList);
    void initProgress(int count);
    void updateProgress(int value);
    void closeProgress();
    void updateStatus(QString);
};

#endif // FILESCANNER_H
