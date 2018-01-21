#ifndef SCENEPROXYMODEL_H
#define SCENEPROXYMODEL_H
#include <QDate>
#include <QSortFilterProxyModel>
#include "definitions.h"
class SceneProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SceneProxyModel(QObject *parent = 0);
    void removeFilters(void);
    QString filterActor() const { return nameFilter; }
    void setFilterActor(const QString &name);
    QString filterCompany() const { return companyFilter;   }
    void setFilterCompany(const QString &company);
    QString filterTag() const { return tagFilter; }
    void setFilterTag(const QString &tag);
    int filterQuality() const { return qualityFilter;   }
    void setFilterQuality(const int &quality);
    void setFilterFilename(const QString &);
    void setFilterID(const int &id);
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
private:
    void setFilter(QString text);
    enum FilterKey{ NAME, COMPANY, TAG, QUALITY, NONE};
    QString getCellData(int row, int column, const QModelIndex &sourceParent) const;
    void setDefaultValues();
    bool filenameMatchesFilter(int row, const QModelIndex & index) const;
    bool nameMatchesFilter(int row, const QModelIndex &index) const;
    bool companyMatchesFilter(QString company) const;
    bool tagMatchesFilter(QString tag) const;
    bool qualityMatchesFilter(QString qualityString) const;
    QString currentFilter;
    QString nameFilter, companyFilter, tagFilter, fileFilter;
    int qualityFilter, idFilter;
    FilterKey key;
};

#endif // SCENEPROXYMODEL_H
