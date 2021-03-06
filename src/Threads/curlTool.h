#ifndef CURLTOOL_H
#define CURLTOOL_H
#include "definitions.h"
#include "Biography.h"
#include "SceneList.h"
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
namespace Curl{
    enum Task    { MAKE_BIO, UPDATE_BIO, SCANNING_BIOS };

}

//enum CurlRequest { IAFD, Freeones, Photo, Filmography };
class DownloadThread : public QObject, public QRunnable{
    Q_OBJECT
public:
    DownloadThread(QString name);
    DownloadThread(ActorPtr a);
    ~DownloadThread();
    void run();

private:
    void            makeActor           (QString name);
    QString name, html, photo;
    ActorPtr actor;
    Curl::Task task;
signals:
    void sendActor(ActorPtr);
    void finished();
};

class curlTool : public QObject
{
    Q_OBJECT
public:
    curlTool();
    ~curlTool();
    /** Photos */
    bool                    generateThumbnail   (ScenePtr s);
    bool                    thumbnailExists     (ScenePtr s);
    static bool             getFreeonesData     (QString name, class Biography &bio);
    static bool             getIAFDData         (QString name, class Biography &bio);
    static bool             getFreeonesData     (QString name, class Biography &bio, QString &html);
    static bool             getIAFDData         (QString name, class Biography &bio, QString &html);
    static QString          getHTML             (Website w, QString name);
    static QString          bioSearchIAFD       (QString html, QString key);
    static QString          bioSearchFO         (QString html, QString key);
    static QMap<QString,QString> parseBioTags   (QString html, Website site);
    static bool             downloadHeadshot    (ActorPtr a, QString html);
    static QString          downloadHeadshot    (QString name);
    static ActorPtr         downloadActor       (QString name, Biography bio);

public slots:
    void            downloadPhoto       (ActorPtr a);
    void            updateBio           (ActorPtr a);
//  void            downloadPhotos      (ActorList a);
    void            updateBios          (ActorList a);
    void            makeNewActors       (QStringList nameList);
    void            db_to_ct_buildActors(QStringList);
    void            pd_to_ct_getActor   (QString);
    void            apv_to_ct_getProfile(QString);
private slots:
    void            receiveActor        (ActorPtr);
    void            downloadThreadComplete();

private:
    static QString         getHeadshotLink     (QString);
    static bool            wget                (QString, QString);
    void            resetCounters();
    int             threadsStarted, threadsFinished;
    int             index;
    int             additions;
    QString         userAppData, dataPath, headshotPath, thumbnailPath;
    bool            keepRunning;
    ActorPtr        currentActor;
    QMutex          curlMx, threadMx;
    QThreadPool     *threadPool;
    QStringList     nameList;
    ActorList       actorList;
    DownloadThread  *downloadThread;
    Curl::Task            currentTask;


signals:
    void            ct_to_db_storeActors(ActorList);
    void            ct_to_pd_sendActor(ActorPtr);
    void            ct_to_apv_sendActor(ActorPtr);
    void            finishedProcessing(bool);
    void            startProgress(QString, int);
    void            updateProgress(int);
    void            updateStatus(QString);
    void            closeProgress(QString);
    void            updateFinished(ActorList);
    void            showError(QString);
    void            updateSingleProfile(ActorPtr);
};
#endif // CURLTOOL_H
