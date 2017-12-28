#ifndef CURLTOOL_H
#define CURLTOOL_H
#include "definitions.h"
#include <QMutex>
#include <QRunnable>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QThreadPool>
#define DATAPATH        "/usr/local/scenequery"
#define APP_DATA_PATH   "/Users/derby/Library/Application Support/SceneQuery"
#define IAFD_URL        "http://www.iafd.com/person.rme"
#define FREEONES_URL    "http://www.freeones.ca/html"
#define HEADSHOT_PATH   "headshots"
#define THUMBNAIL_PATH  "thumbs"
enum Website { IAFD, Freeones };

QString         downloadHeadshot    (QString name);
QString         headshotName    (QString name);
bool            getFreeonesData     (QString name, class Biography *bio);
bool            getIAFDData         (QString name, class Biography *bio);
class Biography freeones            (QString);
class Biography iafd                (QString);
bool            wget                (QString, QString);
QString         getHTML             (Website w, QString name);
QString         bioSearchIAFD       (QString html, QString key);
QString         bioSearchFO         (QString html, QString key);
QString         getHeadshotName     (QString actorName);
//enum CurlRequest { IAFD, Freeones, Photo, Filmography };
class DownloadThread : public QObject, public QRunnable{
    Q_OBJECT
public:
    DownloadThread(QString name);
    ~DownloadThread();
    void run();
private:
    void            makeActor           (QString name);
    QString name, html, photo;
    ActorPtr actor;
signals:
    void sendActor(ActorPtr);
    void finished();
};

class curlTool : public QThread
{
    Q_OBJECT
public:
    curlTool();
    ~curlTool();
    void run();
    /** Photos */
    bool            generateThumbnail   (ScenePtr s);
    bool     thumbnailExists     (ScenePtr s);


public slots:
    void            downloadPhoto       (ActorPtr a);
//    void            updateBio           (ActorPtr a);
//  void            downloadPhotos      (ActorList a);
//  void            updateBios          (ActorList a);
    void            updateBios          (QStringList nameList);
    void            stopThread          (void);
private slots:
    void            receiveActor        (ActorPtr);
    void            downloadThreadComplete();
private:
    int             threadsStarted, threadsFinished;
    int             index;
    int             additions;
    QString         userAppData, dataPath, headshotPath, thumbnailPath;
    bool            keepRunning;
    QMutex          curlMx;
    QThreadPool     threadPool;
    QStringList     nameList;
    ActorList       actorList;
    QString         getThumbnailFormat  (QString sceneFilename);
    QString         formatImageName     (QString filename, QString photoExtension="jpeg");
    bool            getDimensions       (QString name, class Height &height, int &weight);
signals:
    void            finishedProcessing(bool);
    void            startProgress(QString, int);
    void            updateProgress(int);
    void            closeProgress(QString);
    void            updateFinished(ActorList);
    void            showError(QString);
};
#endif // CURLTOOL_H
