#ifndef GENERICFUNCTIONS_H
#define GENERICFUNCTIONS_H
#include "definitions.h"
#include <QStringList>
#include <QDateTime>
#include <QDate>
#include <QDir>
QStringList getEntryList(QString path, QDir::Filter typeFilter, QStringList nameFilters);
QString     listToString(QStringList);
QString     system_call(QString);
QStringList getRatingList(void);
bool        system_call_blocking(QString command, QStringList args);
bool        system_call_blocking(QString command, QStringList args, QString &output);
ActorList   MapToList(ActorMap actors);
QImage      scaleImage(QString file, int targetHeight, Qt::TransformationMode mode=Qt::FastTransformation);
QImage      scaleImage(QImage source, int height);
//std::string qtos    (QString q)     {   return q.toStdString();                 }
//QString     stoq    (std::string s) {   return QString::fromStdString(s);       }
bool        empty   (QString s);
bool        full    (QString s);
bool        nonzero (int i);
bool        nonzero (double d);
bool        valid   (QDate d);
bool        valid   (QDateTime d);
#endif // GENERICFUNCTIONS_H
