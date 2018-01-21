#include <QtWidgets>
#include <QRegExp>
#include "SceneProxyModel.h"

SceneProxyModel::SceneProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent){
    setDefaultValues();
}
void SceneProxyModel::setDefaultValues(){
    nameFilter="";
    companyFilter="";
    fileFilter = "";
    tagFilter="";
    qualityFilter=-1;
}
/// Filter Setting
void SceneProxyModel::removeFilters(){
    setDefaultValues();
    setFilter(".*");
}
void SceneProxyModel::setFilter(QString text){
//    invalidateFilter();
    this->setFilterRegExp(text);
    this->setFilterFixedString(text);
}

void SceneProxyModel::setFilterActor(const QString &name){
    this->nameFilter = name;
    setFilter(name);
}
void SceneProxyModel::setFilterCompany(const QString &company){
    this->companyFilter = company;
    setFilter(company);
}
void SceneProxyModel::setFilterTag(const QString &tag){
    this->companyFilter = tag;
    setFilter(tag);
}
void SceneProxyModel::setFilterQuality(const int &quality){
    this->qualityFilter = quality;
    invalidateFilter();
}
void SceneProxyModel::setFilterID(const int &id){
    this->idFilter = id;
}
void SceneProxyModel::setFilterFilename(const QString &searchTerm){
    this->fileFilter = searchTerm;
    setFilter(searchTerm);
}

bool SceneProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const{
    return (nameMatchesFilter(source_row, source_parent) && filenameMatchesFilter(source_row, source_parent));
}

QString SceneProxyModel::getCellData(int row, int column, const QModelIndex &sourceParent) const{
    return sourceModel()->data(sourceModel()->index(row, column, sourceParent)).toString();
}

bool SceneProxyModel::filenameMatchesFilter(int row, const QModelIndex &index) const{
    bool match = false;
    if (fileFilter == ".*" || fileFilter.isEmpty()){
        match = true;
    } else {
        QString filenameData = getCellData(row, SCENE_PATH_COLUMN, index);
        match = filenameData.contains(fileFilter);
     //   qDebug("'%s' %s match filter '%s'", qPrintable(filenameData), (match ? "Does" : "Doesn't"), qPrintable(fileFilter));
    }
    return match;
}

bool SceneProxyModel::nameMatchesFilter(int row, const QModelIndex &currIndex) const{
    bool match = false;
    if (nameFilter == ".*"  || nameFilter.isEmpty()){
        match = true;
    } else {
        QString mainName = getCellData(row, SCENE_NAME_COLUMN, currIndex);
        QString featNames = getCellData(row, SCENE_FEATURED_COLUMN, currIndex);
        if(mainName.contains(nameFilter) || featNames.contains(nameFilter)){
            match = true;
        }
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
bool SceneProxyModel::tagMatchesFilter(QString tag) const{
    QRegularExpression rx(tagFilter);
    QRegularExpressionMatch match = rx.match(tag);
    return match.hasMatch();
}

bool SceneProxyModel::companyMatchesFilter(QString company) const{
    QRegularExpression rx(companyFilter);
    QRegularExpressionMatch m = rx.match(company);
    return m.hasMatch();
}
