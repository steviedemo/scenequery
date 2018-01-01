#ifndef SCENEPROXYMODEL_H
#define SCENEPROXYMODEL_H
#include <QDate>
#include <QSortFilterProxyModel>

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
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    //bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
private:
    void setFilter(QString text);
    enum FilterKey{ NAME, COMPANY, TAG, QUALITY, NONE};
    bool getFilterData(FilterKey, int &column, QString &filterText) const;
    const char *enumToString() const;
    void setDefaultValues();
    bool nameMatchesFilter(QString name) const;
    bool companyMatchesFilter(QString company) const;
    bool tagMatchesFilter(QString tag) const;
    bool qualityMatchesFilter(QString qualityString) const;
    QString nameFilter, companyFilter, tagFilter;
    int qualityFilter;
    FilterKey key;
};

#endif // SCENEPROXYMODEL_H
