#ifndef GENERICFUNCTIONS_H
#define GENERICFUNCTIONS_H

#include <QStringList>
#include <QDateTime>
#include <QDate>
#include <QDir>
QStringList getEntryList(QString path, QDir::Filter typeFilter, QStringList nameFilters);
QString     listToString(QStringList);
QString     system_call(QString);
//std::string qtos    (QString q)     {   return q.toStdString();                 }
//QString     stoq    (std::string s) {   return QString::fromStdString(s);       }
bool        empty   (QString s);
bool        full    (QString s);
bool        nonzero (int i);
bool        nonzero (double d);
bool        valid   (QDate d);
bool        valid   (QDateTime d);
#endif // GENERICFUNCTIONS_H
