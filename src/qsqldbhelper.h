#ifndef QSqlDBHELPER_H
#define QSqlDBHELPER_H
#include <QtSql>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QDebug>
#include <QSharedPointer>
#include "definitions.h"
class QSqlDBHelper
{
public:
    QSqlDBHelper(const char *driver="QPSQL");
    ~QSqlDBHelper();
    bool        connect(const QString& server,
                            const QString& databaseName,
                            const QString& userName,
                            const QString& password);
    void disconnect();
    QString     getLastError(void);
    QSharedPointer<QSqlQuery>   newQuery(void);
    QSharedPointer<QSqlDatabase>getDbPointer(void);
    bool        query           (QString queryString, QStringList queryArgs);
    bool        makeQuery       (QString queryString, QStringList queryArgs, QSqlQuery *query);
    bool        sceneQuery      (QString queryString, QStringList queryArgs, QSqlQuery* query);
    bool        actorQuery      (QString queryString, QStringList queryArgs, QSqlQuery* query);
    bool        runQuery        (QSqlQuery *query);
    bool        executeInsert   (QSqlQuery *query);
    bool        executeUpdate   (QSqlQuery *query);
    bool        executeDelete   (QSqlQuery *query);
    int         selectRowCountResult(QSqlQuery* query);
private:
    QSqlDatabase db;
    bool checkError(QSqlQuery *query, bool result);
};

#endif // QSqlDBHELPER_H
