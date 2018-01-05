#include "sql.h"
#include "Actor.h"
#include "Scene.h"
#include "filenames.h"
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
#define trace(); qDebug("%s::%s::%d", __FILE__, __FUNCTION__, __LINE__);
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


/** Called:  FileScanner ---> SQL
 *      Add Scanned Scenes to Database, and then gets the ID of each entry and store it back into the Scene Object
 *  Calls:  SQL ---> MainWindow
 *      Returns the Scene Objects to the Main Window.
 */
void SQL::db_to_mw_sendScenes(SceneList list){
    sqlConnection *sql = new sqlConnection();
    foreach(ScenePtr s, list){
        Query q = s->toQuery();
        if (sql->execute(q, SQL_INSERT)){
            QString statement = QString("SELECT id FROM scenes WHERE filename=%1 AND filepath=%2").arg(Query::sqlSafe(s->getFilename())).arg(Query::sqlSafe(s->getFolder()));
            const std::string statement_str = statement.toStdString();
            if (sql->execute(statement_str)){
                pqxx::result R = sql->getResult();
                if (R.size() > 0){
                    try{
                        pqxx::result::const_iterator i = R.begin();
                        s->setID(i["id"].as<int>());
                    } catch (std::exception &e){
                        qWarning("Error Setting ID for Scene %s", qPrintable(s->getFilepath()));
                    }
                } else {
                    qWarning("Error Retrieving ID for Scene %s", qPrintable(s->getFilepath()));
                }
            } else {
                qWarning("Error Retrieving %s from database", qPrintable(s->getFilepath()));
            }
        } else {
            qWarning("Error Storing %s to the database", qPrintable(s->getFilepath()));
        }
    }
    qDebug("Scenes stored to database");
    emit db_to_mw_sendScenes(list);
}

/** Called:  curlTool ---> SQL
 *       Add built Actors to Database, and then gets the ID of each entry and store it back into the Actor Object
 *  Calls:  SQL ---> MainWindow
 *      Returns the Actor Objects to the Main Window.
 */
void SQL::ct_to_db_storeActors(ActorList list){
    sqlConnection *sql = new sqlConnection();
    foreach(ActorList a, list){
        Query q = a->toQuery();
        if (sql->execute(q, SQL_INSERT)){
            QString statement = QString("SELECT id FROM actors WHERE name=%1").arg(Query::sqlSafe(a->getName()));
            const std::string statement_str = statement.toStdString();
            if (sql->execute(statement_str)){
                pqxx::result R = sql->getResult();
                if (R.size() > 0){
                    try{
                        pqxx::result::const_iterator i = R.begin();
                        a->setID(i["id"].as<int>());
                    } catch (std::exception &e){
                        qWarning("Error Setting ID for Actor %s", qPrintable(a->getName()));
                    }
                } else {
                    qWarning("Error Retrieving ID for Actor %s", qPrintable(a->getName()));
                }
            } else {
                qWarning("Error Retrieving %s from database", qPrintable(a->getName()));
            }
        } else {
            qWarning("Error Storing %s to the database", qPrintable(a->getName()));
        }
    }
    qDebug("Actors stored to database");
    db_to_mw_sendActors(list);
}

/** Called:  FileScanner ---> SQL.
 *      Receive list of names picked from scanned scenes, and assemble a list
 *      of names that aren't already in the database.
 *  Calls:  SQL ----> curlTool
 *      Build Actor object for each new name.
 */
void SQL::fs_to_db_checkNames(QStringList nameList){
    QStringList newNames;
    foreach(QString name, nameList){
        if (!hasActor(name)){
            newNames << name;
        }
    }
    emit db_to_ct_buildActors(newNames);
}


void SQL::drop(ActorPtr a){
    qDebug("Deleting '%s' from the actors table", qPrintable(a->getName()));
    QString name = QString("%1").arg(a->getName());
    name.replace('\'', "\'\'");
    name.prepend('\'');
    name.append('\'');
    QString statement = QString("DELETE from actors where NAME = %1").arg(name);
    sqlConnection sql(statement.toStdString());
    sql.execute();
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
    if (!i.empty()){
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
            count.idx++;
            emit updateProgress(count.idx);
            mx.unlock();
        }
    } else {
        qWarning("Empty result iterator passed");
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

void SQL::threaded_profile_photo_scaler(ActorPtr a){
    qDebug("Setting profile photo for %s", qPrintable(a->getName()));
    QString path_to_photo = getProfilePhoto(a->getName());
    mx.lock();
    QPixmap photo = QPixmap(path_to_photo);
    mx.unlock();
    a->setScaledProfilePhoto(QVariant(photo.scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT)));
    mx.lock();
    emit updateProgress(++initIndex);
    mx.unlock();
}


void SQL::initialize(){
    int index = 0;
    this->actors = {};
    this->scenes = {};
    qDebug("Initiliazing Item Lists");
    sqlConnection *sceneSql = new sqlConnection(QString("SELECT * FROM scenes"));
    sqlConnection *actorSql = new sqlConnection(QString("SELECT * FROM actors"));
    if (!sceneSql->execute()){
        qWarning("Error Loading Scenes");
    }
    if (!actorSql->execute()){
        qWarning("Error Loading Actors");
    }
    pqxx::result actorResult = actorSql->getResult();
    pqxx::result sceneResult = sceneSql->getResult();
    int totalItems = actorResult.size() + sceneResult.size();
    emit startProgress(QString("Reading %1 items in from database").arg(totalItems), (2*actorResult.size() + sceneResult.size()));
    qDebug("Adding %lu Scenes", sceneResult.size());
    for (pqxx::result::const_iterator S = sceneResult.begin(); S != sceneResult.end(); ++S){
        ScenePtr s = ScenePtr(new Scene(S));
        scenes.push_back(s);
        emit updateProgress(++index);

    }
    qDebug("Adding %lu Actors", actorResult.size());
    for (pqxx::result::const_iterator A = sceneResult.begin(); A != sceneResult.end(); ++A){
        ActorPtr a = ActorPtr(new Actor(A));
        actors.push_back(a);
        emit updateProgress(++index);
    }
    qDebug("Building Scaled Actor Profile Photos");
    QFutureSynchronizer<void>sync;
    for(int i = 0; i < actors.size(); ++i){
        ActorPtr a = actors.at(i);
        sync.addFuture(QtConcurrent::run(this, &SQL::threaded_profile_photo_scaler, a));
    }
    sync.waitForFinished();
    qDebug("Initialization Complete");
    emit initializationFinished(actors, scenes);
    emit closeProgress();
}

/** \brief Load scenes from the database into the list of scenes passed
 *  \param SceneList &scenes:   Scenes already in list.
 */
void SQL::load(SceneList sceneList){
    this->scenes = {};
    if (!sceneList.isEmpty()){
        this->scenes = sceneList;
    }
    count.reset();
    qDebug("Loading Scenes From Database...");
    sqlConnection *sql = new sqlConnection(QString("SELECT * FROM scenes"));
    if (!sql->execute()){
        qWarning("Error Retrieving List of Scenes from the database");
        return;
    }
    emit updateStatus("Loading Scenes from Database");
   // qDebug("SQL Statement Executed");
    pqxx::result resultList = sql->getResult();
  //  qDebug("Getting Result");
    int total = resultList.size();
    emit startProgress(QString("Reading in %1 Scenes from database").arg(total), total);
    ///Crashes somewhere after this.
    for (pqxx::result::const_iterator i = resultList.begin(); i != resultList.end(); ++i){
        loadScene(i);
    }
    emit closeProgress("Scenes Finished Loading");
    delete sql;
    emit sendResult(scenes);
}

void SQL::load(QVector<QSharedPointer<Actor>> actors){
    this->actors = {};
    if (!actors.isEmpty()){
        this->actors = actors;
    }
    count.reset();
    sqlConnection *sql = new sqlConnection(QString("SELECT * FROM actors"));
    qDebug("Loading Actors");
    if (!sql->execute()){
        qWarning("Error Loading List of Actors");
        return;
    }
    pqxx::result resultList = sql->getResult();
    int total = resultList.size();
    emit startProgress(QString("Reading in %1 Actors from database").arg(total), total);
    for (pqxx::result::const_iterator i = resultList.begin(); i != resultList.end(); ++i){
        loadActor(i);
    }
    emit closeProgress("Actors Loaded");
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

bool SQL::hasActor(QString name){
    bool found = false;
    sqlConnection *sql = new sqlConnection(QString("SELECT FROM actors WHERE (NAME LIKE %1)").arg(Query::sqlSafe(name)));
    if ((queryRan = sql->execute())){
        found = sql->foundMatch();
    }
    sql->disconnect();
    delete sql;
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
    Query query = A->toQuery();
    std::string statement = query.toPqxxUpdate("actors");
    //qDebug("SQL Statement: '%s'", statement.c_str());
    sqlConnection *sql = new sqlConnection(statement);
    if (!sql->execute()){
        qWarning("Error Updating %s", name);
        emit updateStatus(QString("Error Updating %1").arg(A->getName()));
    } else {
      //  qDebug("%s Updated!", name);
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

