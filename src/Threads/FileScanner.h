#ifndef FILESCANNER_H
#define FILESCANNER_H
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
#include <QThreadPool>
#include "SceneList.h"

class MiniScanner : public QObject, public QRunnable{
    Q_OBJECT
public:
    explicit MiniScanner(const QString &path) : root(path), infoList({}){}
    ~MiniScanner(){}
    void run();
private:
    QString root;
    QFileInfoList infoList;

signals:
    void done(QFileInfoList);
};

class FileScanner : public QThread
{
    Q_OBJECT
public:
    FileScanner(const QString &path);
    FileScanner(const QStringList &paths);
    ~FileScanner();
    bool setPath(QString);
    void run();
    bool generateThumbnail(ScenePtr s);
    static QFileInfoList   recursiveScan   (const QFileInfo &rootFolder);

public slots:
    void scanForActors(SceneList, ActorList);
    void db_to_fs_receiveUnsavedScenes(QFileInfoList);
    void stopThread();
private slots:
    void miniscanComplete(QFileInfoList);
private:
    void            getFiles            (void);
    QString         getThumbnailFormat  (QString sceneFilename);
    QString         formatImageName     (QString filename, QString photoExtension="jpeg");
    bool            getDimensions       (QString name, class Height &height, int &weight);
    SceneList       makeScenes      (QFileInfoList fileList);
    void            parseScene      (QFileInfo currentFile);
    QStringList     getNames        (SceneList list);
    ActorList parseActorList(SceneList);
    void getActorsFromScene(ScenePtr s);
    QString scanDir;
    QStringList rootFolders;
    ActorList actorList;
    SceneList sceneList;
    QStringList nameList;
    QThreadPool *threadPool;
    int threadsStarted, threadsComplete;
    int index;
    int added;
    bool keepRunning, updateListReceived;
    QMutex subMx, threadMx, fileListMx;
    QFileInfoList scanFiles, foundFiles;
    bool waitingOnCurlThread;
signals:
    /** Scan Process **/
    void fs_to_db_storeScenes(SceneList);
    void fs_to_db_checkScenes(QFileInfoList);
    void fs_to_db_checkNames(QStringList);

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
