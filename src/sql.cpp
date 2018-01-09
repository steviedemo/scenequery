#include "sql.h"
#include "Actor.h"
#include "Scene.h"
#include "filenames.h"
#include "sceneParser.h"
#include "sqlconnection.h"
#include <pqxx/result.hxx>
#include <pqxx/transaction.hxx>
#include <pqxx/connection.hxx>
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
/** Destructor */
SQL::~SQL(){}

/** Main Loop */
void SQL::run(){
    this->keepRunning = true;
    qDebug("SQL Thread Started!");
    while (keepRunning){
        sleep(1);
    }
    qDebug("SQL Thread Stopped!");
}

/** Stop the Thread's Main Event Loop */
void SQL::stopThread(){
    this->keepRunning = false;
}
/** Get a Printable form of the given Query Type enum */
const char *SQL::toString(queryType t){
    if (t == SQL_UPDATE){
        return "UPDATE";
    } else if (t == SQL_INSERT){
        return "INSERT";
    }
    return "SELECT";
}

/** Delete and then re-add a table to the database */
bool SQL::dropTable(Database::Table table){
    sqlConnection sql;
    if (table == Database::SCENE){
        qDebug("Dropping Scenes Table");
        sql.setQuery("DROP TABLE scenes");
    } else if (table == Database::ACTOR){
        qDebug("Dropping Actors Table");
        sql.setQuery("DROP TABLE actors");
    }
    return sql.execute();
}


bool SQL::makeTable(Database::Table table){
    QString queryString("");
    if (table == Database::ACTOR){
        qDebug("Making new Actor Table");
        queryString = ADB_TABLE;
    } else if (table == Database::SCENE) {
        qDebug("Making new Scene Table");
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
    sqlConnection sql(queryString);
    return sql.execute();
}

/** \brief Get a list of unique Companies from the scene table */
QStringList SQL::getCompanyList(){
    return getDistinctValueList("scenes", "company");
}

/** \brief Get a List of Unique values from a given table. */
QStringList SQL::getDistinctValueList(QString tableName, QString fieldName){
    QStringList list;
    std::string statement = QString("SELECT DISTINCT %1 FROM %2 ORDER BY %1").arg(fieldName).arg(tableName).toStdString();
    std::string field = fieldName.toStdString();
    sqlConnection sql(statement);
    if (sql.execute()){
        pqxx::result r = sql.getResult();
        for (pqxx::result::const_iterator entry = r.begin(); entry != r.end(); ++entry){
            if (!entry[field].is_null()){
                list << QString::fromStdString(entry[field].as<std::string>());
            }
        }
    }
    return list;
}

/** \brief Get the entry ID Associated with the actor who's name is passed.
 *          If the actor isn't in the table, return '-1'
 */
int SQL::getActorID(QString name){
    int id = -1;
    QString sqlStatement = QString("SELECT id FROM actors WHERE name=%1").arg(Query::sqlSafe(name));
    try{
        sqlConnection sql(sqlStatement.toStdString());
        if (sql.execute()){
            pqxx::result r = sql.getResult();
            for (size_t rownum=0; rownum < 1 && rownum < r.size(); ++rownum){
                const pqxx::result::tuple row = r[rownum];
                for(size_t colnum=0; colnum < 1 && colnum < row.size(); ++colnum){
                    QString id_string(row.at("id").c_str());
                    bool converted = false;
                    int temp = id_string.toInt(&converted);
                    if (converted && temp > 0){
                        id = temp;
                    }
                }
            }
            if (id < 0){
                qWarning("Unable to get ID for '%s'", qPrintable(name));
            }
        } else {
            qWarning("Failed to execute query '%s'", qPrintable(sqlStatement));
        }
    } catch (std::exception &e) {
        qWarning("Caught Exception while attempting to run query: '%s':\n\t%s", qPrintable(sqlStatement), e.what());
    }
    return id;
}

int SQL::getSceneID(QString filepath, QString filename){
    int id = 0;
    QString sqlFilename = QString("%1").arg(Query::sqlSafe(filename));
    QString sqlFilepath = QString("%1").arg(Query::sqlSafe(filepath));
    std::string statement = QString("SELECT id FROM scenes WHERE filepath=%1 AND filename=%2").arg(sqlFilepath).arg(sqlFilename).toStdString();
    try{
        sqlConnection sql(statement);
        if (sql.execute()){
            pqxx::result r = sql.getResult();
            if (r.size() > 0){
                pqxx::result::const_iterator entry = r.begin();
                if (!entry["id"].is_null()){
                    id = entry["id"].as<int>();
                    qDebug("Got ID of %d for Scene %s", id, qPrintable(filename));
                } else {
                    qWarning("ID is null for %s", qPrintable(filename));
                }
            } else {
                qWarning("No Results found for file with name '%s' & path '%s'", qPrintable(filename), qPrintable(filepath));
            }
        } else {
            qWarning("Failed to execute query '%s'", statement.c_str());
        }
    } catch (std::exception &e){
        qWarning("Caught Exception while attempting to retrieve entry ID for file: %s:\n\t%s", statement.c_str(), e.what());
    }
    return id;
}

/** Called:  FileScanner ---> SQL
 *      Add Scanned Scenes to Database, and then gets the ID of each entry and store it back into the Scene Object
 *  Calls:  SQL ---> MainWindow
 *      Returns the Scene Objects to the Main Window.
 */
void SQL::fs_to_db_storeScenes(SceneList list){
    sqlConnection *sql = new sqlConnection();
    emit startProgress(QString("Adding %1 Scanned Scenes to the Database").arg(list.size()), list.size());
    int saves(0), index(0);
    foreach(ScenePtr s, list){
        Query q = s->toQuery();
        bool queryRan = false, sceneStored = false;
        sceneStored = hasScene(s, queryRan);
        if (!queryRan){
            qWarning("Error Checking if %s is already in the database", qPrintable(s->getFilename()));
        } else {
            if (!sceneStored){
                std::string statement = q.toPqxxInsert("scenes");
                if (sql->execute(statement)){
                    saves++;
                    int id = getSceneID(s->getFolder(), s->getFilename());
                    if (id > 0){
                        s->setID(id);
                    }
                } else {
                    qWarning("Error Storing %s to the database", qPrintable(s->getFullpath()));
                }
            } else {
                int id = 0;
                id = getSceneID(s->getFolder(), s->getFilename());
                s->setID(id);
            }
        }
        emit updateProgress(++index);
    }
    emit closeProgress();
    emit updateStatus(QString("%1/%2 Scenes were new, and stored to the database").arg(saves).arg(list.size()));
    delete sql;
    qDebug("%d Scenes stored to database", saves);
    emit db_to_mw_sendScenes(list);
}


/** Called:  curlTool ---> SQL
 *       Add built Actors to Database, and then gets the ID of each entry and store it back into the Actor Object
 *  Calls:  SQL ---> MainWindow
 *      Returns the Actor Objects to the Main Window.
 */
void SQL::ct_to_db_storeActors(ActorList list){
    int adds(0), index(0);
    qDebug("SQL Thread Received list of %d Actors to store to the Database", list.size());
    emit startProgress(QString("Adding %1 Actors to the Database").arg(list.size()), list.size());
    sqlConnection *sql = new sqlConnection();
    foreach(ActorPtr a, list){
        Query q = a->toQuery();
        std::string statement = q.toPqxxInsert("actors");
        if (sql->execute(statement)){
            adds++;
            qDebug("Successfully Added %s to the Database", qPrintable(a->getName()));
            QString statement = QString("SELECT id FROM actors WHERE name=%1").arg(Query::sqlSafe(a->getName()));
            const std::string statement_str = statement.toStdString();
            if (sql->execute(statement_str)){
               int id = getActorID(a->getName());
               if (id > 0){
                   a->setID(id);
               }
            } else {
                qWarning("Error Retrieving %s from database", qPrintable(a->getName()));
            }
        } else {
            qWarning("Error Storing %s to the database", qPrintable(a->getName()));
        }
        emit updateProgress(++index);
    }
    emit closeProgress(QString("%1/%2 Actors Stored to the Database").arg(adds).arg(list.size()));
    qDebug("SQL Thread sending %d actors to the Main Window", adds);
    emit db_to_mw_sendActors(list);
    delete sql;
}

void SQL::pd_to_db_saveActor(ActorPtr a){
    bool inDatabase = false;
    if (!hasActor(a->getName())){
        std::string statement = a->toQuery().toPqxxInsert("actors");
        sqlConnection sql(statement);
        if (sql.execute()){
            qDebug("Successfully added %s to the database", qPrintable(a->getName()));
            inDatabase = true;
        } else {
            qWarning("Error Adding actor to database");
        }
    } else {
        inDatabase = true;
    }
    if (inDatabase){
        int id = getActorID(a->getName());
        if (id > 0){
            a->setID(id);
        } else {
            qWarning("Error Setting ID for '%s'", qPrintable(a->getName()));
        }
    }
    emit db_to_pd_sendBackWithID(a);
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
    int removals = 0;

    QVector<QPair<QString, QString>> deleteList = {};
    sqlConnection *sql = new sqlConnection();
    if (sql->execute("SELECT filepath, filename FROM scenes")){
        int index = 0;
        pqxx::result r = sql->getResult();
        int fileCount = (int)r.size();
        emit startProgress(QString("Checking for the existance of %1 Files...").arg(fileCount), fileCount);
        for (pqxx::result::const_iterator entry = r.begin(); entry != r.end(); ++entry){
            try{
                if (!entry["filepath"].is_null()){
                    QString path = QString::fromStdString(entry["filepath"].as<std::string>());
                    if (!entry["filename"].is_null()){
                        QString name = QString::fromStdString(entry["filename"].as<std::string>());
                        QString absolutePath = QString("%1/%2").arg(path).arg(name);
                        QFile file(absolutePath);
                        if (!file.exists()){
                            QPair<QString,QString> currentFile;
                            currentFile.first = path;
                            currentFile.second = name;
                            deleteList.push_back(currentFile);
                            qDebug("Adding '%s' to list of files to remove from the database", qPrintable(absolutePath));
                        }
                    }
                }
            } catch (std::exception &e){
                qWarning("Exception Caught parsing row %d while purging scene table: %s", index, e.what());
            }
            ++index;
            emit updateProgress(index);
        }
        emit closeProgress();
        index = 0;
        emit startProgress(QString("Removing %1 Scenes From the Database").arg(deleteList.size()), deleteList.size());
        if (deleteList.size() > 0){
            qDebug("Deleting %d scenes from the database", deleteList.size());
            for(int i = 0; i < deleteList.size(); ++i){
                QPair<QString,QString> curr = deleteList.at(i);
                try{
                    QString statement = QString("DELETE FROM scenes WHERE filepath = %1 AND filename = %2").arg(Query::sqlSafe(curr.first)).arg(Query::sqlSafe(curr.second));
                    if (sql->execute(statement.toStdString())){
                        qDebug("%s Successfully removed", qPrintable(curr.second));
                        ++removals;
                    }
                } catch (std::exception &e){
                    qWarning("Exception Caught removing %s/%s from the database: %s", qPrintable(curr.first), qPrintable(curr.second), e.what());
                }
                ++index;
                emit updateProgress(index);
            }
            qDebug("%d/%d Entries Successfully removed from the Scene Database", removals, deleteList.size());
        }
        emit closeProgress();
        emit updateStatus(QString("%1/%2 Scenes were Successfully Removed from the Database").arg(removals).arg(deleteList.size()));
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
    sqlConnection sql(s->toQuery(), SQL_UPDATE);
    if (!sql.execute()){
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
    sqlConnection sceneSql(QString("SELECT * FROM scenes"));
    sqlConnection actorSql(QString("SELECT * FROM actors"));
    if (!sceneSql.execute()){
        qWarning("Error Loading Scenes");
    }
    if (!actorSql.execute()){
        qWarning("Error Loading Actors");
    }
    pqxx::result actorResult = actorSql.getResult();
    pqxx::result sceneResult = sceneSql.getResult();
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
    emit closeProgress();
    emit initializationFinished(actors, scenes);
}

/** \brief Load scenes from the database into the list of scenes passed
 *  \param SceneList &scenes:   Scenes already in list.
 */
void SQL::loadScenes(){
    this->scenes = {};
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

void SQL::loadActors(){
    this->actors = {};
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
    QPair<QString,QString> file = s->getFile();
    sqlConnection *sql = new sqlConnection(QString("SELECT FROM scenes WHERE (FILEPATH = %1 AND FILENAME = %2)").arg(Query::sqlSafe(file.first)).arg(Query::sqlSafe(file.second)));
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
    if (sql->execute()){
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
    sqlConnection sql(QString("SELECT * FROM scenes WHERE ID = %1").arg(S->getID()));
    if (!sql.execute()){
        qWarning("Error Running Query: %s", qPrintable(sql.getQuery()));
        return false;
    } else {
        stored = sql.foundMatch();
    }

    sql.clear();
    queryType operation = (stored ? SQL_UPDATE : SQL_INSERT);
    sql.setQuery(S->toQuery(), operation);
    success = sql.execute();
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

