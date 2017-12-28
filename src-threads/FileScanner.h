#ifndef FILESCANNER_H
#define FILESCANNER_H
#include "FilePath.h"
#include "definitions.h"
// Qt Stuff
#include <QObject>
#include <QStandardItem>
#include <QString>
#include <QThread>
#include <QStringList>
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
    void scanForActors(SceneList, ActorList);
    void scanFolder(QString rootPath);
    void receiveUpdatedActors(ActorList);
    void stopThread();
private:
    QFileInfoList   recursiveScan   (QFileInfo rootFolder);
    SceneList       makeScenes      (QFileInfoList fileList);
    void            parseScene      (QFileInfo currentFile);
    QStringList     getNames        (SceneList list);
    ActorList parseActorList(SceneList);
    void getActorsFromScene(ScenePtr s);
    QString scanDir;
    QVector<FilePath> files;
    ActorList actors;
    SceneList scenes;
    QStringList newNames;
    int index;
    int added;
    bool keepRunning;
    QMutex mx;
    bool waitingOnCurlThread;
signals:
    /// Progress Updating
    void startProgress(QString, int);
    void closeProgress(QString);
    void updateProgress(int value);
    void updateStatus(QString);
    /// Passing data back to the main thread.
    void showError(QString);
    void scanComplete(SceneList, QStringList);
    void scanComplete(ActorList);
    /// Request to curl thread.
    void updateBios(ActorList);
};

#endif // FILESCANNER_H
