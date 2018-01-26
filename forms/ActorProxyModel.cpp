#include "ActorProxyModel.h"
#include "genericfunctions.h"
ActorProxyModel::ActorProxyModel(QObject *parent):
    QSortFilterProxyModel(parent),
    anythingRx(".*")
{
    clearFilters();
}
void ActorProxyModel::clearFilters(){
    this->sceneCountFilter = -1;
    this->nameRx = anythingRx;
    this->hairRx = anythingRx;
    this->skinRx = anythingRx;
    this->nameFilter        = ".*";
    this->hairFilter        = ".*";
    this->skinFilter        = ".*";
    this->heightFilter      = Height();
    this->weightFilter      = -1;
    this->sceneCountFilter  = -1;
    this->ageFilter         = -1;
    this->tattooFilter      = NOT_SET;
    this->piercingFilter    = NOT_SET;
}

QString ActorProxyModel::getCellData(int row, int col, const QModelIndex &currIndex) const{
    return sourceModel()->data(sourceModel()->index(row, col, currIndex)).toString();
}
//************* Checking cells against a filter **********************/
bool ActorProxyModel::filterMatches_name(int row, const QModelIndex &index) const{
    bool match = true;
    if(!filterMatchesAnything(nameFilter)){
        QString text = getCellData(row, ACTOR_NAME_COLUMN);
        QRegularExpression rx(nameFilter);
        QRegularExpressionMatch m = rx.match(text);
        match = m.hasMatch();
    }
    return match;
}

bool ActorProxyModel::filterMatches_hair(int row, const QModelIndex &index) const{
    bool match = true;
    if(!filterMatchesAnything(hairFilter)){
        QString text = getCellData(row, ACTOR_HAIR_COLUMN);
        QRegularExpression rx(hairFilter);
        QRegularExpressionMatch m = rx.match(text);
        match = m.hasMatch();
    }
    return match;
}

bool ActorProxyModel::filterMatches_skin(int row, const QModelIndex &index) const{
    bool match = true;
    if(!filterMatchesAnything(skinFilter)){
        QString text = getCellData(row, ACTOR_ETH_COLUMN);
        QRegularExpression rx(skinFilter);
        QRegularExpressionMatch m = rx.match(text);
        match = m.hasMatch();
    }
    return match;
}
bool ActorProxyModel::filterMatches_age(int row, const QModelIndex &index) const{
    bool match = true;
    if(!filterMatchesAnything(ageOp) || (ageFilter < 0)){
        bool ok = false;
        QString text = getCellData(row, ACTOR_AGE_COLUMN);
        if (!text.isEmpty()){
            int num = text.toInt(&ok);
            if (ok){
                match = compare(ageOp, num, ageFilter);
            }
        }
    }
    return match;
}
bool ActorProxyModel::filterMatches_weight(int row, const QModelIndex &index) const{
    bool match = true;
    if(!filterMatchesAnything(weightOp) || (weightFilter < 0)){
        bool ok = false;
        QString text = getCellData(row, ACTOR_WEIGHT_COLUMN);
        if (!text.isEmpty()){
            int num = text.toInt(&ok);
            if (ok){
                match = compare(weightOp, num, weightFilter);
            }
        }
    }
    return match;
}

bool ActorProxyModel::filterMatches_height(int row, const QModelIndex &index) const{
    bool match = true;
    if(!filterMatchesAnything(heightOp) || heightFilter.isValid()){
        QString text = getCellData(row, ACTOR_HEIGHT_COLUMN);
        if (!text.isEmpty()){
            Height h(text);
            if (h.isValid()){
                match = compare(heightOp, h, heightFilter);
            }
        }
    }
    return match;
}

bool ActorProxyModel::filterMatches_tattoos(int row, const QModelIndex &index) const{
    return filterMatchesTriState(tattooFilter, getCellData(row, ACTOR_TATTOO_COLUMN));
}

bool ActorProxyModel::filterMatches_piercings(int row, const QModelIndex &index) const{
    return filterMatchesTriState(piercingFilter, getCellData(row, ACTOR_PIERCING_COLUMN));
}

bool ActorProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const{
    bool name       = filterMatches_name     (source_row, source_parent);
    bool hair       = filterMatches_hair     (source_row, source_parent);
    bool skin       = filterMatches_skin     (source_row, source_parent);
    bool height     = filterMatches_height   (source_row, source_parent);
    bool weight     = filterMatches_weight   (source_row, source_parent);
    bool tattoos    = filterMatches_tattoos  (source_row, source_parent);
    bool piercings  = filterMatches_piercings(source_row, source_parent);
    //bool scenes     = filterMatches_scenes   (source_row, source_parent);
    bool age        = filterMatches_age      (source_row, source_parent);
    return (name && hair && skin && height && weight && tattoos && piercings && age);
}
