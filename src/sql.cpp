#include "sql.h"
#include "qsqldbhelper.h"
#include "Actor.h"
#include "Scene.h"
#include <QObjectUserData>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QDate>
#include <QString>
#include <QVector>

SQL::SQL(QString connectionName){
    // Start Postgresql
    QString cmd = "if [[ -z $(postgres -V) ]]; then pg_ctl -D /usr/local/var/postgres start && brew services start postgresql; fi";
    if (system(qPrintable(cmd)) == EXIT_FAILURE){
        qWarning("Error Starting Postgres Server!");
    }
    this->connectionName = connectionName;
    this->db = QSqlDatabase::addDatabase("QPSQL", connectionName);
    db.setHostName(HOST);
    db.setUserName(USERNAME);
    db.setPassword(PASSWORD);

}

bool SQL::connect(){
    bool success = false;
    if (db.isOpen()){
        success = true;
    }  else if ((success = db.open())){
        qDebug("Successfully opened connection '%s' to database", qPrintable(connectionName));
    } else {
        qWarning("Error Opening Connection '%s' to Database", qPrintable(connectionName));
    }
    return success;
}

SQL::~SQL(){
    db.close();
    db.removeDatabase(this->connectionName);
}
// Check if a record exists in the database
bool SQL::hasMatch(QSqlQuery *q){   return (countMatches(q) > 0);   }


// Use a Query String and a Parameter List to assemble a QSqlQuery object.
QSharedPointer<QSqlQuery> SQL::assembleQuery(QString s, QStringList args, bool &ok){
    QSharedPointer<QSqlQuery> query = QSharedPointer<QSqlQuery>(new QSqlQuery(db));
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
 * Converting strings to the appropriate Syntax for SQL queries.
 *------------------------------------------------------------------*/
QString sqlSafe(QDateTime d){
    QString s("");
    if (valid(d)){
        s=d.toString("'yyyy-MM-dd'");
    }
    return s;
}

QString sqlSafe(QDate d)    {
    QString s("");
    if (d.isValid() && !d.isNull())
        s=d.toString("'yyyy-MM-dd'");
    return s;
}

QString sqlSafe(FilePath f){
    return sqlSafe(f.absolutePath());
}

QString sqlSafe(QString s){
    QString sql("");
    if (s.isEmpty())
        return sql;
    int f_offset = 0, b_offset = 0;
    sql.append("'");
    try{
        if (s.startsWith("'"))  {   f_offset = 1;   }
        if (s.endsWith("'"))    {   b_offset = 1;   }
        for (int i = 0; i < s.size(); ++i){
            QChar c = s.at(i);
            if (c == '\'')                    // escape apostrophes
                sql.append("''");
            else if (c != '\"' && c != ';') // don't include semicolons or quotation marks
                sql.append(c);
         }
         int lastPosition = sql.size() - 1;
         while(sql.at(lastPosition) == '\''){
            sql.remove(lastPosition);
            lastPosition = sql.size() - 1;
         }
    } catch (std::exception &e) {
        qDebug("Caught Exception While Creating Sql Safe String from '%s':\t%s", qPrintable(s), e.what());
    } catch (...) {
        qDebug("Caught Unknown Exception while creating SQL safe string from '%s'", qPrintable(s));
    }
    sql.append("'");
    return sql;
}
void    sqlAppend(QString &fields, QString &values, QStringList &list, QString fieldname, QString itemvalue){
    if (!itemvalue.isEmpty() && !itemvalue.isNull() && itemvalue != "0" && itemvalue != "0.0"){
        if (list.size() > 0)
            fields.append(',');
        fields.append(QString(" %1 = ?").arg(fieldname));
        values.append(", ?");
        list << itemvalue;
    }
}

void    sqlAppend(QString &fields, QStringList &list, QString fieldname, QString itemvalue){
    if (!itemvalue.isEmpty() && !itemvalue.isNull() && itemvalue != "0" && itemvalue != "0.0"){
        if (list.size() > 0)
            fields.append(',');
        fields.append(QString(" %1 = ?").arg(fieldname));
        list << itemvalue;
    }
}

void sqlAppend(QString &fields, QString &values, QString name, QString item, bool prev){
    if (item != "0" && full(item)){
        if (prev){
            fields.append(',');
            values.append(',');
        }
        fields.append(name.toUpper());
        values.append(item);
    }
}

void sqlAppend(QString &fields, QString title, QString item, bool &prev){
    if (item != "0" && item != "0.0" && full(item)){
        if (prev)
            fields.append(',');
        fields.append(QString(" %1=%2").arg(title).arg(item));
        prev = true;
    }
}

const char *toString(Database::queryType q){
    if (q == Database::UPDATE)
        return "Update";
    else if (q == Database::INSERT)
        return "Add";
    else if (q == Database::REQUEST)
        return "Get";
    else
        return "???";
}



/*------------------------------------------------------------------
 * Clear unused items out of the table.
 *------------------------------------------------------------------*/
void purgeScenes(void){
    QSqlDBHelper sql;
    if (!sql.connect(HOST, SCENE_DB, USERNAME, PASSWORD)){
        qWarning("Unable to Connect to database - Cannot Purge Scenes");
    } else {
        QSqlTableModel model;
        model.setTable(SCENE_DB);
        model.setFilter("*");
        if (!model.select()){
            qWarning("Unable to populate QSqlTableModel with Scenes from the Database!");
            return;
        } else {
#warning unfinished function
        }
    }
}


/*------------------------------------------------------------------
 * Retrieving a Vector of Items from the records in a table.
 *------------------------------------------------------------------*/
/** \brief Load scenes from the database into the list of scenes passed
 *  \param SceneList &scenes:   Scenes already in list.
 */
void loadSceneList(SceneList &scenes){
    QSqlDBHelper sql;
    if (!sql.connect(HOST, SCENE_DB, USERNAME, PASSWORD)){
        qWarning("Unable to Connect to database - Cannot Load Scenes");
        return;
    }
    // Set up the Table Model
    QSqlQueryModel model;
    model.setQuery("SELECT * FROM scenes");
    for (int i = 0; i < model.rowCount(); ++i){
        QSharedPointer<Scene> scenePointer = QSharedPointer<Scene>(new Scene(model.record(i)));
        scenes.push_back(scenePointer);
    }
    sql.disconnect();
}

void loadActorList(ActorList &actors){
    QSqlDBHelper sql;
    if (!sql.connect(HOST, ACTOR_DB, USERNAME, PASSWORD)){
        qWarning("Unable to Connect to database - Cannot Load Actors");
        return;
    }
    // Set up the Table Model
    QSqlQueryModel model;
    model.setQuery("SELECT * FROM actors");
    for (int i = 0; i < model.rowCount(); ++i){
        QSharedPointer<Actor> temp = QSharedPointer<Actor>(new Actor(model.record(i).value("name").toString()));
        if (!actors.contains(temp)){
            QSharedPointer<Actor> actorPointer = QSharedPointer<Actor>(new Actor(model.record(i)));
            actors.push_back(actorPointer);
        }
    }
    sql.disconnect();
}

/*------------------------------------------------------------------
 * ADDING/UPDATING a single item.
 *------------------------------------------------------------------*/
/** \brief Update Database with single item, or update item from database
 *  \param QSharedPointer<Scene> S: Scene object
 *  \param queryType:   Update, or retrieve.
 */
bool SQL::sceneSql(QSharedPointer<Scene> S, Database::queryType type){
    QString queryString("");
    QStringList queryArgs;
    const char *name = qPrintable(S->getFile().getName());
    bool success = false, workToDo = false;
    qDebug("Attempting to %s %s", toString(type), name);
    // Get Query Syntax.
    if (type == Database::UPDATE){
        workToDo = S->sqlUpdate(queryString, queryArgs);
    } else if (type == Database::INSERT){
        workToDo = S->sqlInsert(queryString, queryArgs);
    } else {
        qCritical("Invalid Query Type Requested for Actor %s: %s", name, toString(type));
    }
    // If there's nothing to do, return true.
    if (!workToDo){
        qDebug("Nothing to %s for %s", toString(type), name);
        return true;
    } else {
        // Run Query
        bool error = false, ok = false;
        db.transaction();
        QSharedPointer<QSqlQuery> q = assembleQuery(queryString, queryArgs, ok);
        if (ok){
            if (q->exec()){
                qDebug("Committing Query");
                if (db.commit()){
                    qDebug("%s of '%s' Successful.", toString(type), name);
                } else {
                    error = true;
                }
            } else {
                db.rollback();
                error = true;
            }
        } else {
            error = true;
        }
        if (error){
            qWarning("Error Encountered While Trying to %s '%s'", toString(type), name);
        } else {
            success = true;
        }
    }
    return success;
}

bool SQL::actorSql(QSharedPointer<Actor> A, Database::queryType type){
    QString queryString("");
    QStringList queryArgs;
    const char *name = qPrintable(A->getName());
    bool success = false, workToDo = false;

    qDebug("Attempting to %s %s", toString(type), name);
    // Get Query Syntax.
    if (type == Database::UPDATE){
        workToDo = A->sqlUpdate(queryString, queryArgs);
    } else if (type == Database::INSERT){
        workToDo = A->sqlInsert(queryString, queryArgs);
    } else {
        qCritical("Invalid Query Type Requested for Actor %s: %s", name, toString(type));
    }
    // If there's nothing to do, return true.
    if (!workToDo){
        qDebug("Nothing to %s for %s", toString(type), name);
        return true;
    } else {
        // Run Query
        bool ok = false, error = false;
        db.transaction();
        QSharedPointer<QSqlQuery> q = assembleQuery(queryString, queryArgs, ok);
        if (ok){
            if (q->exec()){
                if (db.commit()){
                    success = true;
                } else {
                    error = true;
                }
            } else {
                db.rollback();
                error = true;
            }
        } else {
            error = true;
        }
    }
    return success;
}


/*------------------------------------------------------------------
 * ADDING/UPDATING from items in a vector
 *------------------------------------------------------------------*/
bool SQL::hasScene(ScenePtr s){
    db.transaction();
    QString queryString = QString("SELECT * FROM scenes WHERE actor1 = %1 AND title = %2 AND size = %3 AND height = %4 AND length = %5 ").arg(s->getActor(0)).arg(s->getTitle()).arg(s->getSize()).arg(s->getHeight()).arg(s->getLength());
    QSqlQuery q;
    q.exec(queryString);
    return (q.size() > 0);
}

void SQL::updateDatabase(SceneList sceneList){
    operation_count count;
    foreach(QSharedPointer<Scene> S, sceneList){
        count.idx++;
        // General Variables needed in most cases
        QSqlQuery query(db);
        // Check if record is in the list
        if (query.exec(QString("SELECT FROM ACTORS WHERE (NAME LIKE %1)").arg(S->getFile().getPath()))){
            if (query.size() == 0){    // Insert into table
                count.added += (sceneSql(S, Database::INSERT) ? 1 : 0);
            } else {    // Update or Leave alone
                count.updated += (sceneSql(S, Database::UPDATE) ? 1 : 0);
            }
        } else {
            qCritical("Error Querying Table for Scene '%s'", qPrintable(S->getFile().getPath()));
        }
    }
    qDebug("\n\nAdded %d new Scenes.\nUpdated %d Existing Records.\n%d/%d Records from list used to modify table.\n", count.added, count.updated, count.total(), sceneList.size());
}

void SQL::updateDatabase(ActorList actorlist){
    operation_count count;
    foreach(QSharedPointer<Actor> A, actorlist){
        count.idx++;
        // General Variables needed in most cases
        QSqlQuery query(db);
        // Check if record is in the list
        if (query.exec(QString("SELECT FROM ACTORS WHERE (NAME LIKE %1)").arg(A->getName()))){
            if (query.size() == 0){    // Insert into table
                count.added += (actorSql(A, Database::INSERT) ? 1 : 0);
            } else {    // Update or Leave alone
                count.updated += (actorSql(A, Database::UPDATE) ? 1 : 0);
            }
        } else {
            qCritical("Error Querying Table for Actor '%s'", qPrintable(A->getName()));
        }
    }
    qDebug("\n\nAdded %d new Actors.\nUpdated %d Existing Records.\n%d/%d Records from list used to modify table.\n", count.added, count.updated, count.total(), actorlist.size());
}

// Run a SELECT query and count the number of records that match.
int SQL::countMatches(QSqlQuery *q){
    int matches = 0;
    if (!connect()){
        return 0;
    } else {
        q->exec();
        matches = q->size();
    }
    return matches;
}

// Run an UPDATE or INSERT query on the database
bool SQL::modifyDatabase(QSqlQuery *q){
    bool result = false;
    if (!db.isValid()){
        qCritical("Database %s is not valid. Cannot Run Query.", qPrintable(db.databaseName()));
    } else if (!connect()){
        qCritical("Unable to open connection to %s. Cannot Run Query.", qPrintable(db.databaseName()));
    } else if (!q->isValid()) {
        qCritical("Query is invalid.");
    } else {
        db.transaction();
        if (q->exec() && (q->lastError().type() == QSqlError::NoError)){
            result = db.commit();
            qDebug("Successfully Ran Query");
        } else {
            db.rollback();
            qWarning("Error Running Query: %s", qPrintable(q->lastError().text()));
        }
    }
    return result;
}

QSharedPointer<QSqlQuery> SQL::queryDatabase(QString s, QStringList a){
    bool ok = false;
    QSharedPointer<QSqlQuery> q= assembleQuery(s, a, ok);
    if (ok && q->exec()){
        qDebug("Successfully ran query");
    } else {
        qWarning("Error Running Query: %s", qPrintable(db.lastError().text()));
    }
    return q;
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
    db.transaction();
    QSqlQuery q;
    if (q.exec(queryString)){
        success = db.commit();
    }
    return success;
}

