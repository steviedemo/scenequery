#ifndef FILTERSET_H
#define FILTERSET_H
#include "Rating.h"
#include <QDate>
#include <QTime>
#include "definitions.h"

class FilterSet
{
public:
    FilterSet();
    explicit FilterSet(const class SceneProxyModel *);
    void clear();
    bool save(const QString &filepath);
private:
    QString writeLine(QString, QString);
    LogicalOperator durationOp, qualityOp, sizeOp, ratingOp, releasedOp, addedOp;
    QString nameFilter, companyFilter, tagFilter, fileFilter;
    qint64 sizeFilter;
    QTime durationFilter;
    QDate addedFilter, releasedFilter;
    Rating ratingFilter;
    int qualityFilter, idFilter;

};

#endif // FILTERSET_H
