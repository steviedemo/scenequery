#ifndef ACTORPROXYMODEL_H
#define ACTORPROXYMODEL_H
#include "definitions.h"
#include <QSortFilterProxyModel>

#include <QRegularExpression>
class ActorProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum NumberFilterType{ greater_than, less_than, equal_to };
    ActorProxyModel(QObject *parent = 0);
    void clearFilters();
    QString getFilterHair() const;
    QString getFilterEthnicity() const;
    int getFilterSceneCount() const;
    void setFilterHairColor(const QString &hairColor);
    void setFilterEthnicity(const QString &ethnicity);
    void setFilterSceneCount(const NumberFilterType &, const int &count);
    void clearFilterHairColor(void);
    void clearFilterEthnicity(void);
    void clearFilterSceneCount(void);
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
private:
    QRegularExpression hairRx, skinRx, anythingRx;
    QRegularExpressionMatch match;
    int sceneCountFilter;
    NumberFilterType countRelation;
    QString getCellData(int row, int col, const QModelIndex &currIndex) const;
    bool hairMatchesFilter(int row, const QModelIndex &index) const;
    bool ethnicityMatchesFilter(int row, const QModelIndex &index) const;
    bool sceneCountMatchesFilter(int row, const QModelIndex &index) const;
};

#endif // ACTORPROXYMODEL_H
