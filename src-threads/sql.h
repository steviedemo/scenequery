#ifndef SQL_H
#define SQL_H
#include "definitions.h"
#include "sql_definitions.h"
#include "FilePath.h"
#include "sqlconnection.h"
#include <QMutex>
#include <QtSql>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDatabase>
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


class SQL : public QThread {
    Q_OBJECT
public:
    SQL(QString connectionName=DEFAULT_NAME);
    ~SQL();
    void            run();
    static void     purgeScenes(void);
    void            startPostgres();
    static const char *toString         (queryType);
    QueryPtr        queryDatabase       (QString queryText, QStringList args);
    QueryPtr        assembleQuery       (QString queryText, QStringList args, bool &ok);
    void            loadScene           (pqxx::result::const_iterator &i);
    void            loadActor           (pqxx::result::const_iterator &i);
    bool            sceneSql            (ScenePtr S, queryType type);
    bool            actorSql            (ActorPtr A, queryType type);

    // Static Functions
    static void     sqlAppend           (QString &fields, QStringList &list, QString name, QString item);
    static void     sqlAppend           (QString &fields, QString &values, QStringList &list, QString name, QString item);
    static void     sqlAppend           (QString &fields, QString &values, QString name, QString item, bool prev);
    static void     sqlAppend           (QString &fields, QString name, QString item, bool &prev);
    // Hight Level, Threaded Routines
public slots:
    void            saveChanges         (ScenePtr);
    void            updateActor         (ActorPtr);
    void            stopThread          ();
    void            load                (ActorList actors);
    void            load                (SceneList scenes);
    void            store               (ActorList actors);
    void            store               (SceneList scenes);
    bool            hasScene            (ScenePtr s, bool &queryRan);
    bool            hasActor            (ActorPtr a, bool &queryRan);
    bool            makeTable           (Database::Table);
    bool            dropTable           (Database::Table);

protected:
    // Single-Thread helper functions for multi-threaded routines.
    bool            insertOrUpdateActor (ActorPtr);
    bool            insertOrUpdateScene (ScenePtr);
private:
    operation_count count;
    QSqlDatabase db;
    QString connectionName;
    sqlConnection connection;
    QMutex mx;
    SceneList scenes;
    ActorList actors;
    bool keepRunning;

signals:
    void updateStatus(QString status);
    void startProgress(QString, int);
    void updateProgress(int);
    void closeProgress(QString);
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
