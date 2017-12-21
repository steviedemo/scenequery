#include "QSqlDBhelper.h"

QSqlDBHelper::QSqlDBHelper(const char* driver){
    this->db = QSqlDatabase( QSqlDatabase::addDatabase(driver));
}

QSqlDBHelper::~QSqlDBHelper(){
}

bool QSqlDBHelper::connect(const QString &server, const QString &databaseName, const QString &userName, const QString &password)
{
    bool success = false;
    db.setConnectOptions();
    db.setHostName(server);
    db.setDatabaseName(databaseName);
    db.setUserName(userName);
    db.setPassword(password);

    if (db.open()){
        success = true;
    }
    return success;
}

QString QSqlDBHelper::getLastError(void){
    QString msg("");
    msg = db.lastError().text();
    return msg;
}

/** \brief Disconnect from the database */
void QSqlDBHelper::disconnect()
{
    qDebug() << "Disconnected from Database!";
    db.close();
}

QSharedPointer<QSqlQuery> QSqlDBHelper::newQuery(void){
    QSharedPointer<QSqlQuery> query = QSharedPointer<QSqlQuery>(new QSqlQuery(db));
    return query;
}

QSharedPointer<QSqlDatabase> QSqlDBHelper::getDbPointer(){
    return QSharedPointer<QSqlDatabase>(&db);
}

bool QSqlDBHelper::query(QString queryString, QStringList args)
{
    bool success = false;
    QSqlQuery query(db);
    if (makeQuery(queryString, args, &query)){
        query.setForwardOnly(true);
        success = runQuery(&query);
    }
    return success;
}

bool QSqlDBHelper::sceneQuery(QString queryString, QStringList queryArgs, QSqlQuery *query){
    bool success = false;
    if (makeQuery(queryString, queryArgs, query)){
        query->setForwardOnly(true);
        success = runQuery(query);
    }
    return success;
}


bool QSqlDBHelper::actorQuery(QString queryString, QStringList args, QSqlQuery *query){
    bool success = false;
    if (makeQuery(queryString, args, query)){
        query->setForwardOnly(true);
        success = runQuery(query);
    }
    return success;
}

bool QSqlDBHelper::makeQuery(QString queryString, QStringList args, QSqlQuery *query){
    bool success = false;
    query->setForwardOnly(true);
    // Make the query
    if (!query->prepare(queryString)){
        qWarning("Error Making Query: %s", qPrintable(db.lastError().text()));
    } else {
        // Bind the parameters
        for (int i = 0; i < args.size(); ++i) {
            query->addBindValue(args.at(i));
        }
        success = true;
    }
    return success;
}

bool QSqlDBHelper::runQuery(QSqlQuery *query){
    bool result = false;
    if (!db.isValid()){
        qCritical("Database %s is not valid. Cannot Run Query.", qPrintable(db.databaseName()));
    } else if (!db.isOpen()){
        qCritical("Database %s not open. Cannot Run Query.", qPrintable(db.databaseName()));
    } else if (!query->isValid()){
        qWarning("Query is Invalid. Cannot Run on %s Database.", qPrintable(db.databaseName()));
    } else {
        db.transaction();
        bool result = query->exec();
        if (result && query->lastError().type() == QSqlError::NoError) {
            db.commit();
            qDebug("Successfully ran Query.");
            result = true;
        } else {
            qWarning("Error Running Query: %s", qPrintable(query->lastError().text()));
            db.rollback();
        }
    }
    return result;
}

bool QSqlDBHelper::executeInsert(QSqlQuery *query)  { return runQuery(query); }
bool QSqlDBHelper::executeDelete(QSqlQuery *query)  { return runQuery(query); }
bool QSqlDBHelper::executeUpdate(QSqlQuery *query)  { return runQuery(query); }


int QSqlDBHelper::selectRowCountResult(QSqlQuery *query)
{
    bool queryRes = query->exec();
    if (query->lastError().type() != QSqlError::NoError || !queryRes)
    {
        qDebug() << query->lastError().text();
        return -1;
    }
    int recordCount = 0;
    while(query->next())
    {
        qDebug() << "Field 1: " << query->value(0).toString();
        qDebug() << "Filed 2: " << query->value(1).toString();
        ++recordCount;
    }
    return recordCount;
}
