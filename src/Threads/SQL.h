#ifndef SQL_H
#define SQL_H
#include "definitions.h"
#include "sql_definitions.h"
#include "sqlconnection.h"
#include "SceneList.h"
#include <QMutex>
/*
#include <QtSql>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDatabase>
*/
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QMap>


#define DEFAULT_NAME "default"
struct operation_count {
    int idx, added, updated, failed;
    operation_count() : idx(0), added(0), updated(0), failed(0){}
    void reset()    {   idx = 0; added = 0; updated = 0; failed = 0;    }
    int total()     {   return added + updated + failed;                }
    int successes() {   return (added + updated);                       }
    void addInsert(){   idx++;  added++;    }
    void addUpdate(){   idx++;  updated++;  }
    void addFailed(){   idx++;  failed++;   }
};


class SQL : public QObject {
    Q_OBJECT
public:
    SQL()   {   startServer();  }
    ~SQL()  {                   }
    void            startServer();
    static const char *toString         (queryType);
    QueryPtr        queryDatabase       (QString queryText, QStringList args);
    QueryPtr        assembleQuery       (QString queryText, QStringList args, bool &ok);
    void            loadScene           (pqxx::result::const_iterator &i);
    void            loadActor           (pqxx::result::const_iterator &i);
    bool            sceneSql            (ScenePtr S, queryType type);
    bool            actorSql            (ActorPtr A, queryType type);
    int             getActorID          (QString name);
    int             getSceneID          (QString filepath, QString filename);
    QStringList     getCompanyList      (void);
    QStringList     getDistinctValueList(QString tableName, QString fieldName);
    // Static Functions
    static void     sqlAppend           (QString &fields, QStringList &list, QString name, QString item);
    static void     sqlAppend           (QString &fields, QString &values, QStringList &list, QString name, QString item);
    static void     sqlAppend           (QString &fields, QString &values, QString name, QString item, bool prev);
    static void     sqlAppend           (QString &fields, QString name, QString item, bool &prev);
    // Hight Level, Threaded Routines
public slots:
    void            saveChanges         (ScenePtr);
    void            updateActor         (ActorPtr);
    void            drop                (ActorPtr);
    void            dropActor           (QString name);
    void            loadActors          ();
    void            loadScenes          ();
    void            store               (ActorList actors);
    void            store               (SceneList scenes);
    bool            hasScene            (ScenePtr s, bool &queryRan);

    bool            hasActor            (ActorPtr a, bool &queryRan);
    bool            makeTable           (Database::Table);
    bool            dropTable           (Database::Table);
    void            purgeScenes         (void);

    void            fs_to_db_storeScenes(SceneList);
    void            fs_to_db_checkNames (QStringList);
    void            fs_to_db_checkScenes(QFileInfoList);
    void            ct_to_db_storeActors(ActorList);
    void            pd_to_db_saveActor  (ActorPtr);

protected:
    // Single-Thread helper functions for multi-threaded routines.
    bool            insertOrUpdateActor (ActorPtr);
    bool            insertOrUpdateScene (ScenePtr);
    bool            hasActor            (QString);
private:
    QMutex mx;
    operation_count actorCount, sceneCount;
    SceneList scenes;
    ActorList actors;

signals:
    void db_to_mw_sendActors(ActorList);
    void db_to_mw_sendScenes(SceneList);
    void db_to_ct_buildActors(QStringList);
    void db_to_pd_sendBackWithID(ActorPtr);
    void db_to_fs_sendUnsavedScenes(QFileInfoList);
    void initializationFinished(ActorList, SceneList);
    void updateStatus(QString status);
    void startProgress(QString, int);
    void updateProgress(int);
    void closeProgress(QString);
    void closeProgress();
    void sendPurgeList(QVector<int>);

    void sendResult(bool);
    void sendResult(ActorList);
    void sendResult(SceneList);
    void operationComplete();
    void showError(QString);
    void showSuccess(QString);
    void sceneSaveComplete();
    void actorSaveComplete();
};


#endif // SQL_H
