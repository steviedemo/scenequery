#ifndef GENERICFUNCTIONS_H
#define GENERICFUNCTIONS_H
#include "definitions.h"
#include <QStringList>
#include <QDateTime>
#include <QDate>
#include <QDir>
template<class T>
bool compare(const LogicalOperator &op, const T &i, const T &j){
    bool p = false;
    if (op == EQUAL){
        p = (i == j);
    } else if (op == LESSER_THAN){
        p = (i < j);
    } else if (op == LESSER_OR_EQUAL){
        p = (i <= j);
    } else if (op == GREATER_THAN){
        p = (i > j);
    } else if (op == GREATER_OR_EQUAL) {
        p = (i >= j);
    } else if (op == NOT_EQUAL){
        return (i != j);
    } else if (op == NOT_SET){
        p = true;
        qWarning("Operator not set when comparing two items");
    }
    return p;
}
bool        filterMatchesTriState(const TriState &t, const QString &s) const;
bool        filterMatchesAnything(const QString &s) const;
bool        filterMatchesAnything(const LogicalOperator &op) const  {   return (op == NOT_SET); }
bool        filterMatchesAnything(const TriState &op) const         {   return (op == NOT_SET); }
bool        filterMatchesAnything(const int &i) const               {   return i > -1;          }
QString     toString(const LogicalOperator &op);
LogicalOperator fromString(const QString &s);
QStringList getEntryList(QString path, QDir::Filter typeFilter, QStringList nameFilters);
QString     listToString(QStringList);
QString     system_call(QString);
QStringList getRatingList(void);
bool        system_call_blocking(QString command, QStringList args);
bool        system_call_blocking(QString command, QStringList args, QString &output);
ActorList   MapToList(ActorMap actors);
QImage      scaleImage(QString file, int targetHeight, Qt::TransformationMode mode=Qt::FastTransformation);
QImage      scaleImage(QImage source, int height);
int         getScaledWidth(const QImage &source, const int height);
QSize       getScaledSize(const QImage &source, const int height);

//bool        compare(const LogicalOperator &op, int, int);
//bool        compare(const LogicalOperator &op, qint64, qint64);
//bool        compare(const LogicalOperator &op, QDate &d1, QDate &d2);
//bool        compare(const LogicalOperator &op, QTime &t1, QTime &t2);
//bool        compare(const LogicalOperator &op, class Rating &r1, class Rating &r2);

//std::string qtos    (QString q)     {   return q.toStdString();                 }
//QString     stoq    (std::string s) {   return QString::fromStdString(s);       }
bool        empty   (QString s);
bool        full    (QString s);
bool        nonzero (int i);
bool        nonzero (double d);
bool        valid   (QDate d);
bool        valid   (QDateTime d);
bool        valid   (QString s);
#endif // GENERICFUNCTIONS_H
