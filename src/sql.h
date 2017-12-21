#ifndef SQL_H
#define SQL_H
#include <QMutex>
#include <QtSql>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include "definitions.h"
#include "FilePath.h"
struct operation_count {
    int idx, added, updated;
    operation_count() :
        idx(0), added(0), updated(0){}
    int total(){ return added + updated;    }
    void reset(){
        idx = 0;
        added = 0;
        updated = 0;
    }
    void addInsert(){
        idx++;
        added++;
    }
    void addUpdate(){
        idx++;
        updated++;
    }
};

class SQL : public QObject {
    Q_OBJECT
public:
    SQL(QString connectionName="default");
    ~SQL();
    bool connect        (void);
    static void purgeScenes(void);
    int  countMatches   (QSqlQuery *q);
    bool hasMatch       (QSqlQuery *q);
    bool hasScene       (ScenePtr s, bool &queryRan);
    bool hasActor       (ActorPtr a, bool &queryRan);
    bool insertOrUpdateActor(ActorPtr);
    bool insertOrUpdateScene(ScenePtr);
    bool modifyDatabase (QSqlQuery *q);
    QSharedPointer<QSqlQuery> queryDatabase(QString queryText, QStringList args);
    QSharedPointer<QSqlQuery> assembleQuery(QString queryText, QStringList args, bool &ok);

    void updateDatabase (QVector<QSharedPointer<class Actor>> actorList);
    void updateDatabase (QVector<QSharedPointer<class Scene>> sceneList);
    bool makeTable      (Database::Table);
    bool dropTable      (Database::Table);
    bool sceneSql(ScenePtr S, queryType type);
    bool actorSql(ActorPtr A, queryType type);
    // Static Functions
    static void    sqlAppend   (QString &fields, QStringList &list, QString name, QString item);
    static void    sqlAppend   (QString &fields, QString &values, QStringList &list, QString name, QString item);
    static void    sqlAppend   (QString &fields, QString &values, QString name, QString item, bool prev);
    static void    sqlAppend   (QString &fields, QString name, QString item, bool &prev);
    static QString sqlSafe     (QString s);
    static QString sqlSafe     (QDate);
    static QString sqlSafe     (QDateTime d);
    static QString sqlSafe     (int i)              {   return QString("'%1'").arg(i);          }
    static QString sqlSafe     (double d)           {   return QString("'%1'").arg(d);          }
    static QString sqlSafe     (FilePath f);
    static void loadActorList  (QVector<QSharedPointer<class Actor>> &actors);
    static void loadSceneList  (QVector<QSharedPointer<class Scene>> &scenes);
private:
    operation_count count;
    QSqlDatabase db;
    QString connectionName;
    QMutex mx;
signals:
    void startProgress(int total);
    void updateProgress(int);
    void closeProgress();
};

#define ADB_TABLE 	"CREATE TABLE IF NOT EXISTS ACTORS("\
                    "ID				smallserial 	primary key,"\
                    "NAME 			text 			unique not null,"\
                    "ALIASES		text,"\
                    "BIRTHDAY		date,"\
                    "CITY			text,"\
                    "COUNTRY		text,"\
                    "ETHNICITY		text,"\
                    "HEIGHT			integer,"\
                    "WEIGHT			integer,"\
                    "MEASUREMENTS	text,"\
                    "HAIR			text,"\
                    "EYES			text,"\
                    "TATTOOS		text,"\
                    "PIERCINGS		text,"\
                    "PHOTO			text)"
#define SDB_TABLE	"CREATE TABLE IF NOT EXISTS SCENES("\
                    "ID				serial			primary key,"\
                    "FILENAME		text			not null,"\
                    "FILEPATH		text			not null,"\
                    "TITLE			text,"\
                    "COMPANY		text,"\
                    "SERIES			text,"\
                    "SCENE_NO		integer,"\
                    "RATING			text,"\
                    "SIZE			float8,"\
                    "LENGTH			float8,"\
                    "WIDTH			integer,"\
                    "HEIGHT			integer,"\
                    "ADDED			date,"\
                    "CREATED		date,"\
                    "ACCESSED		date,"\
                    "ACTOR1			text,"\
                    "AGE1			integer,"\
                    "ACTOR2			text,"\
                    "AGE2			integer,"\
                    "ACTOR3			text,"\
                    "AGE3			integer,"\
                    "ACTOR4			text,"\
                    "AGE4			integer,"\
                    "URL			text,"\
                    "TAGS			text)"
// Store entries from online filmographies
#define FDB_TABLE	"create table if not exists filmographies("\
                    "ID				serial			primary key,"\
                    "actor			text			not null,"\
                    "title			text			not null,"\
                    "company		text			not null,"\
                    "year			integer			not null,"\
                    "scene			integer,"\
                    "tags			text)"
#define THUMBNAIL_DB	"create table if not exists thumbnails("\
                    "ID 			serial 			primary key,"\
                    "SCENEID 		integer			not null,"\
                    "FILENAME		text			not null,"\
                    "DATEADDED		text,"\
                    "SCENEPATH		text,"\
                    "SCENEFILE 		text,"\
                    "IMAGE			OID)"
#define HEADSHOT_DB	"create table if not exists thumbnails("\
                    "ID 			serial 		primary key,"\
                    "FILENAME		text			not null,"\
                    "DATEADDED		text,"\
                    "NAME 			text,"\
                    "IMAGE			bytes)"

#endif // SQL_H
