#include <QtWidgets>
#include "SceneProxyModel.h"

SceneProxyModel::SceneProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent){
    setDefaultValues();
}
void SceneProxyModel::setDefaultValues(){
    key = NONE;
    nameFilter="";
    companyFilter="";
    tagFilter="";
    qualityFilter=-1;
}
/// Filter Setting
void SceneProxyModel::removeFilters(){
    setDefaultValues();
    setFilter("");
}
void SceneProxyModel::setFilter(QString text){
    invalidateFilter();
    this->setFilterFixedString(text);
}

void SceneProxyModel::setFilterActor(const QString &name){
    setDefaultValues();
    key = NAME;
    this->nameFilter = name;
    setFilter(name);
}
void SceneProxyModel::setFilterCompany(const QString &company){
    setDefaultValues();
    key = COMPANY;
    this->companyFilter = company;
    setFilter(company);
}
void SceneProxyModel::setFilterTag(const QString &tag){
    setDefaultValues();
    key = TAG;
    this->companyFilter = tag;
    setFilter(tag);
}
void SceneProxyModel::setFilterQuality(const int &quality){
    setDefaultValues();
    key = QUALITY;
    this->qualityFilter = quality;
    invalidateFilter();
}

const char *SceneProxyModel::enumToString() const{
    QString s("'No Filter'");
    if (key== NAME){
        s = "'Name'";
    } else if (key == QUALITY){
        s = "'Quality'";
    } else if (key == COMPANY){
        s = "'Company'";
    } else if (key == TAG){
        s = "'tag'";
    }
    return qPrintable(s);
}

bool SceneProxyModel::getFilterData(FilterKey fk, int &column, QString &filterText) const {
    bool applyFilter = true;
    if (fk == COMPANY){
        column = COMPANY_COLUMN;
        filterText = companyFilter;
    } else if (fk == QUALITY){
        column = QUALITY_COLUMN;
        filterText = qualityFilter;
    } else {
        column = 0;
        filterText = "";
        applyFilter = false;
    }
    return applyFilter;
}

/// Filtering
bool SceneProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const{
    int column;
    QString filterText("");
    bool accepted = true;
    if (key == NAME && !nameFilter.isEmpty()){
        accepted = nameMatchesFilter(sourceRow, sourceParent);
    } else if (getFilterData(key, column, filterText)){
        QString data = sourceModel()->data(sourceModel()->index(sourceRow, column, sourceParent)).toString();
        if (filterText == "*" || filterText == ".*"){
            accepted = true;
        } else if (data.isEmpty()){
            accepted = false;
        } else if (filterText.contains(data)){
            accepted = true;
        } else {
            accepted = false;
        }
    }
    return accepted;
}

QString SceneProxyModel::getCellData(int row, int column, const QModelIndex &sourceParent) const{
    return sourceModel()->data(sourceModel()->index(row, column, sourceParent)).toString();
}

bool SceneProxyModel::nameMatchesFilter(int row, const QModelIndex &currIndex) const{
    bool match = false;
    QString mainName = getCellData(row, NAME_COLUMN, currIndex);
    QString featNames = getCellData(row, FEATURED_COLUMN, currIndex);
    if(mainName.contains(nameFilter) || featNames.contains(nameFilter)){
        match = true;
    }
    return match;
}

bool SceneProxyModel::qualityMatchesFilter(QString quality) const{
    bool match = true;
    if (qualityFilter > 0){
        QString filter = QString("%1p").arg(qualityFilter);
        match = (quality==filter);
    }
    return match;
}
bool SceneProxyModel::tagMatchesFilter(QString) const{
    return true;
}
bool SceneProxyModel::companyMatchesFilter(QString company) const{
    bool match = true;
    if (!companyFilter.isEmpty()){
        match = (company == companyFilter);
    }
    return match;
}
