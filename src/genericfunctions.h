#ifndef GENERICFUNCTIONS_H
#define GENERICFUNCTIONS_H

#include <QStringList>
#include <QDateTime>
#include <QDate>
#include <QDir>
QStringList getEntryList(QString path, QDir::Filter typeFilter, QStringList nameFilters);
QString     listToString(QStringList);
QString     system_call(QString);

std::string qtos    (QString q)     {   return q.toStdString();                 }
QString     stoq    (std::string s) {   return QString::fromStdString(s);       }
bool        empty   (QString s)     {   return (s.isEmpty() || s.isNull());     }
bool        full    (QString s)     {   return (!s.isEmpty() && !s.isNull());   }
bool        nonzero (int i)         {   return i > 0;                           }
bool        nonzero (double d)      {   return d > 0.0;                         }
bool        valid   (QDate d)       {   return (d.isValid() && !d.isNull());    }
bool        valid   (QDateTime d)   {   return (d.isValid() && !d.isNull());    }
#endif // GENERICFUNCTIONS_H
