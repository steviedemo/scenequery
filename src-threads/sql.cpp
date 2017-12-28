#include "sql.h"
#include "Actor.h"
#include "Scene.h"
#include "sceneParser.h"
#include "sqlconnection.h"
#include <QFutureSynchronizer>
#include <QtConcurrent>
#include <QtConcurrentRun>
#include <QObjectUserData>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QDate>
#include <QString>
#include <QVector>
#include <QRegularExpression>

using namespace pqxx;

#define START_PSQL "/usr/local/bin/pg_ctl -D /usr/local/var/postgres -l /usr/local/var/postgres/server.log start"
SQL::SQL(QString name){
    // Start Postgresql
    startPostgres();
    this->connectionName = name;
}

void SQL::startPostgres(){
    qDebug("Starting Postgresql..");
    QString output = system_call(START_PSQL);
    qDebug("Output: %s", qPrintable(output));
}

SQL::~SQL(){}

void SQL::run(){
    this->keepRunning = true;
    qDebug("SQL Thread Started!");
    while (keepRunning){
        sleep(1);
    }
    qDebug("SQL Thread Stopped!");
}

void SQL::stopThread(){
    this->keepRunning = false;
}

const char *SQL::toString(queryType t){
    if (t == SQL_UPDATE){
        return "UPDATE";
    } else if (t == SQL_INSERT){
        return "INSERT";
    }
    return "SELECT";
}

bool SQL::dropTable(Database::Table){
    qDebug("Placeholder for drop table function");
    return false;
}



// Use a Query String and a Parameter List to assemble a QSqlQuery object.
QueryPtr SQL::assembleQuery(QString s, QStringList args, bool &ok){
    QueryPtr query = QueryPtr(new QSqlQuery(db));
    query->setForwardOnly(true);
    ok = query->prepare(s);
    if (!ok){
        qWarning("Error Preparing Query %s: %s", qPrintable(s), qPrintable(db.lastError().text()));
    } else {
        QStringListIterator it(args);
        foreach(QString s, args){
            query->addBindValue(s);
        }
        ok = true;
    }
    return query;
}

/*------------------------------------------------------------------
 * Clear unused items out of the table.
 *------------------------------------------------------------------*/
void SQL::purgeScenes(void){
    sqlConnection *sql = new sqlConnection();
    if (sql->execute("SELECT * FROM scenes")){
        pqxx::result r = sql->getResult();
        for(pqxx::result::const_iterator i = r.begin(); i != r.end(); ++i){
            Scene s(i);
            if (!s.exists()){
                /* DROP ENTRY */
            }
        }
    }
    delete sql;
}


/*------------------------------------------------------------------
 * Retrieving a Vector of Items from the records in a table.
 *------------------------------------------------------------------*/
void SQL::loadScene(pqxx::result::const_iterator &i){
    ScenePtr scene = QSharedPointer<Scene>(new Scene(i));
    if (!scenes.contains(scene)){
        mx.lock();
        count.addInsert();
        emit updateStatus(QString("Loaded %1 Scenes").arg(count.added));
        emit updateProgress(count.idx);
        scenes.push_back(scene);
        mx.unlock();
    } else {
        mx.lock();
        emit updateProgress(++count.idx);
        mx.unlock();
    }
}
void SQL::loadActor(pqxx::result::const_iterator &i){
    ActorPtr actor = ActorPtr(new Actor(i));
    if (!actors.contains(actor)){
        mx.lock();
        count.addInsert();
        emit updateProgress(count.idx);
        emit updateStatus(QString("Loaded %1 Actors").arg(count.added));
        actors.push_back(actor);
        mx.unlock();
    } else {
        mx.lock();
        emit updateProgress(++count.idx);
        mx.unlock();
    }
}

void SQL::saveChanges(ScenePtr s){
    sqlConnection *sql = new sqlConnection(s->toQuery(), SQL_UPDATE);
    if (!sql->execute()){
        emit showError("Error Saving changes to database");
    } else {
        emit showSuccess("Changes saved to Database");
    }
}

/** \brief Load scenes from the database into the list of scenes passed
 *  \param SceneList &scenes:   Scenes already in list.
 */
void SQL::load(SceneList scenes){
    this->scenes = scenes;
    count.reset();
    qDebug("Loading Scenes From Database...");
    emit updateStatus("Loading Scenes from Database");
    sqlConnection *sql = new sqlConnection(QString("SELECT * FROM scenes"));
    if (!sql->execute()){
        qWarning("Error Retrieving List of Scenes from the database");
        return;
    }
    pqxx::result resultList = sql->getResult();
    int total = resultList.size();
    emit startProgress(QString("Reading in %1 Scenes from database").arg(total), total);
    QFutureSynchronizer<void> sync;
    for (pqxx::result::const_iterator i = resultList.begin(); i != resultList.end(); ++i){
        sync.addFuture(QtConcurrent::run(this, &SQL::loadScene, i));
    }
    sync.waitForFinished();
    emit closeProgress("Scenes Finished Loading");
    scenes = this->scenes;
    delete sql;
    emit sendResult(scenes);
}

void SQL::load(QVector<QSharedPointer<Actor>> actors){
    this->actors = actors;

    sqlConnection *sql = new sqlConnection(QString("SELECT * FROM actors"));
    qDebug("Loading Actors");
    if (!sql->execute()){
        qWarning("Error Loading List of Actors");
        return;
    }
    pqxx::result resultList = sql->getResult();
    int total = resultList.size();
    emit startProgress(QString("Reading in %1 Actors from database").arg(total), total);
    QFutureSynchronizer<void> sync;
    for (pqxx::result::const_iterator i = resultList.begin(); i != resultList.end(); ++i){
        sync.addFuture(QtConcurrent::run(this, &SQL::loadActor, i));
    }
    sync.waitForFinished();
    emit closeProgress("Scenes Finished Loading");
    actors = this->actors;
    delete sql;
    emit sendResult(actors);
}


/*------------------------------------------------------------------
 * ADDING/UPDATING from items in a vector
 *------------------------------------------------------------------*/
bool SQL::hasScene(ScenePtr s, bool &queryRan){
    bool found = false;
    sqlConnection *sql = new sqlConnection(QString("SELECT FROM scenes WHERE (FILEPATH = %1 AND FILENAME = %2)").arg(Query::sqlSafe(s->getFile().getPath())).arg(Query::sqlSafe(s->getFile().getName())));
    if ((queryRan = sql->execute())){
        found = sql->foundMatch();
    }
    sql->disconnect();
    delete sql;
    emit sendResult(found);
    return found;
}

bool SQL::hasActor(ActorPtr a, bool &queryRan){
    bool found = false;
    sqlConnection *sql = new sqlConnection(QString("SELECT FROM actors WHERE (NAME LIKE %1)").arg(Query::sqlSafe(a->getName())));
    if ((queryRan = sql->execute())){
        found = sql->foundMatch();
    }
    sql->disconnect();
    delete sql;
    emit sendResult(found);
    return found;
}

void SQL::updateActor(ActorPtr A){
    const char *name = qPrintable(A->getName());
    qDebug("Updating Entry for %s", name);
    Query query = A->toQuery();
    sqlConnection *sql = new sqlConnection(query, SQL_UPDATE);
    if (!sql->execute()){
        qWarning("Error Updating %s", name);
        emit updateStatus(QString("Error Updating %1").arg(A->getName()));
    } else {
        qDebug("%s Updated!", name);
        emit updateStatus(QString("Successfully Updated %1").arg(A->getName()));
    }
    delete sql;
}

bool SQL::insertOrUpdateActor(QSharedPointer<Actor> A){
    bool success = false, stored = false;
    sqlConnection *sql = new sqlConnection(QString("SELECT * FROM actors WHERE NAME = %1").arg(Query::sqlSafe(A->getName())));
    if (!sql->execute()){
        qWarning("Error Running Query: %s", qPrintable(sql->getQuery()));
        return false;
    } else {
        stored = sql->foundMatch();
    }
    sql->clear();
    queryType operation = (stored ? SQL_UPDATE : SQL_INSERT);
    sql->setQuery(A->toQuery(), operation);
    success = sql->execute();
    mx.lock();
    if (success && (operation == SQL_INSERT)){
        count.addInsert();
    } else if (success && (operation == SQL_UPDATE)){
        count.addUpdate();
    } else {
        count.addFailed();
    }
    emit updateProgress(count.idx);
    mx.unlock();
    delete sql;
    return success;
}


void SQL::store(ActorList actorList){
    emit startProgress("Storing new actor data in the database", actorList.size());
    QFutureSynchronizer<bool> sync;
    foreach(QSharedPointer<Actor> A, actorList){
        sync.addFuture(QtConcurrent::run(this, &SQL::insertOrUpdateActor, A));
    }
    sync.waitForFinished();
    emit closeProgress("Database Updated with list of Actors");
    qDebug("\n\nAdded %d new Actors.\nUpdated %d Existing Records.\n%d/%d Records from list used to modify table.\n", count.added, count.updated, count.total(), actorList.size());
    emit actorSaveComplete();
}

bool SQL::insertOrUpdateScene(ScenePtr S){
    bool success = false, stored = false;
    sqlConnection *sql = new sqlConnection(QString("SELECT * FROM scenes WHERE ID = %1").arg(S->getID()));
    if (!sql->execute()){
        qWarning("Error Running Query: %s", qPrintable(sql->getQuery()));
        return false;
    } else {
        stored = sql->foundMatch();
    }

    sql->clear();
    queryType operation = (stored ? SQL_UPDATE : SQL_INSERT);
    sql->setQuery(S->toQuery(), operation);
    success = sql->execute();
    mx.lock();
    if (success && (operation == SQL_INSERT)){
        count.addInsert();
    } else if (success && (operation == SQL_UPDATE)){
        count.addUpdate();
    } else {
        count.addFailed();
    }
    emit updateProgress(count.idx);
    mx.unlock();
    delete sql;
    return success;
}

void SQL::store(SceneList sceneList){
    count.reset();
    emit startProgress("Updating Database with list of scenes", sceneList.size());
    QFutureSynchronizer<bool> sync;
    foreach(QSharedPointer<Scene> S, sceneList){
        sync.addFuture(QtConcurrent::run(this, &SQL::insertOrUpdateScene, S));
    }
    sync.waitForFinished();
    emit closeProgress("Finished Updating Database with scene list");
    qDebug("\n\nAdded %d new Scenes.\nUpdated %d Existing Records.\n%d/%d Records from list used to modify table.\n", count.added, count.updated, count.total(), sceneList.size());
    emit sceneSaveComplete();
}


bool SQL::makeTable(Database::Table table){
    QString queryString("");
    bool success = false;
    if (table == Database::ACTOR){
        queryString = ADB_TABLE;
    } else if (table == Database::SCENE) {
        queryString = SDB_TABLE;
    } else if (table == Database::THUMBNAIL) {
        queryString = THUMBNAIL_DB;
    } else if (table == Database::HEADSHOT){
        queryString = HEADSHOT_DB;
    } else if (table == Database::FILMOGRAPHY){
        queryString = FDB_TABLE;
    } else {
        qCritical("Error: Undefined enum value of 'Table'");
        return false;
    }
    sqlConnection *sql = new sqlConnection(queryString);
    if (sql->execute()){
        success = true;
    }
    emit sendResult(success);
    return success;
}

