#include "ActorProxyModel.h"

ActorProxyModel::ActorProxyModel(QObject *parent):
    QSortFilterProxyModel(parent),
    anythingRx(".*")
{
    clearFilters();
}
void ActorProxyModel::clearFilters(){
    this->sceneCountFilter = -1;
    this->hairRx = anythingRx;
    this->skinRx = anythingRx;
}
//******* Ethnicity Filter ******//
void ActorProxyModel::setFilterEthnicity(const QString &ethnicity){
    QString skinFilter = QString(".*%1.*").arg(ethnicity);
    this->skinRx = QRegularExpression(skinFilter);
}
void ActorProxyModel::clearFilterEthnicity(){
    this->skinRx = anythingRx;
}
//****** Hair Color Filter ********//
void ActorProxyModel::setFilterHairColor(const QString &hairColor){
    QString filter = QString(".*%1.*").arg(hairColor);
    this->hairRx = QRegularExpression(filter);
}
void ActorProxyModel::clearFilterHairColor(){
    this->hairRx = anythingRx;
}
//***** Scene Count Filter ****//
void ActorProxyModel::setFilterSceneCount(const NumberFilterType &filterType, const int &sceneCount){
    this->countRelation = filterType;
    this->sceneCountFilter = sceneCount;
}
void ActorProxyModel::clearFilterSceneCount(){
    this->sceneCountFilter = -1;
    this->countRelation = ActorProxyModel::greater_than;
}


QString ActorProxyModel::getCellData(int row, int col, const QModelIndex &currIndex) const{
    return sourceModel()->data(sourceModel()->index(row, col, currIndex)).toString();
}

//************* Checking cells against a filter **********************/
bool ActorProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const{
    bool accepted = hairMatchesFilter(source_row, source_parent);
    accepted = accepted && ethnicityMatchesFilter(source_row, source_parent);
    accepted = accepted && sceneCountMatchesFilter(source_row, source_parent);
    return accepted;
}
bool ActorProxyModel::hairMatchesFilter(int row, const QModelIndex &index) const{
    QString hair = getCellData(row, ACTOR_HAIR_COL, index);
    QRegularExpressionMatch m = hairRx.match(hair);
    return m.hasMatch();
}
bool ActorProxyModel::ethnicityMatchesFilter(int row, const QModelIndex &index) const{
    QString skin = getCellData(row, ACTOR_ETH_COL, index);
    QRegularExpressionMatch m = skinRx.match(skin);
    return m.hasMatch();
}
bool ActorProxyModel::sceneCountMatchesFilter(int row, const QModelIndex &index) const{
    bool matches = true;
    int count = sourceModel()->data(sourceModel()->index(row, ACTOR_SCENE_COL, index)).toInt();
    if (this->countRelation == ActorProxyModel::greater_than){
        matches = (count > sceneCountFilter);
    } else if (this->countRelation == ActorProxyModel::less_than){
        matches = (count < sceneCountFilter);
    } else if (this->countRelation == ActorProxyModel::equal_to){
        matches = (count == sceneCountFilter);
    }
    return matches;
}
