#include <QtWidgets>
#include <QRegExp>
#include "SceneProxyModel.h"
#include "genericfunctions.h"
SceneProxyModel::SceneProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent){
    clearFilters();
}
void SceneProxyModel::clearFilters(){
    durationOp      = NOT_SET;
    qualityOp       = NOT_SET;
    sizeOp          = NOT_SET;
    ratingOp        = NOT_SET;
    releasedOp      = NOT_SET;
    addedOp         = NOT_SET;
    nameFilter      = "";
    companyFilter   = "";
    fileFilter      = "";
    tagFilter       = "";
    qualityFilter   = -1;
    idFilter        = -1;
    sizeFilter      = -1;
    ratingFilter    = Rating("");
    durationFilter  = QTime();
    releasedFilter  = QDate();
    addedFilter     = QDate();
    setFilter(".*");
}
void SceneProxyModel::setFilter(QString text){
//    invalidateFilter();
    this->setFilterRegExp(text);
    this->setFilterFixedString(text);
}

bool SceneProxyModel::filterMatchesAnything(const LogicalOperator &op) const {return (op == NOT_SET);}
bool SceneProxyModel::filterMatchesAnything(const QString &s)          const {return (s.isEmpty() || s == ".*");}

QString SceneProxyModel::getCellData(int row, int column, const QModelIndex &sourceParent) const{
    return sourceModel()->data(sourceModel()->index(row, column, sourceParent)).toString();
}

bool SceneProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const{
    return (
            filterMatches_name      (source_row, source_parent) && \
            filterMatches_filename  (source_row, source_parent) && \
            filterMatches_duration  (source_row, source_parent) && \
            filterMatches_company   (source_row, source_parent) && \
            filterMatches_filesize  (source_row, source_parent) && \
            filterMatches_quality   (source_row, source_parent) && \
            filterMatches_rating    (source_row, source_parent) && \
            filterMatches_release   (source_row, source_parent) \
    );
}

bool SceneProxyModel::filterMatches_filename(int row, const QModelIndex &index) const{
    bool match = false;
    if (fileFilter == ".*" || fileFilter.isEmpty()){
        match = true;
    } else {
        QString filenameData = getCellData(row, SCENE_PATH_COLUMN, index);
        match = filenameData.contains(fileFilter);
    }
    return match;
}

bool SceneProxyModel::filterMatches_name(int row, const QModelIndex &currIndex) const{
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

bool SceneProxyModel::filterMatches_quality(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(qualityOp) && (qualityFilter > 0)){
        QString res = getCellData(row, SCENE_QUALITY_COLUMN, index);
        match = false;
        if (!res.isEmpty()){
            bool ok = false;
            int height = res.remove('p').trimmed().toInt(&ok);
            if (ok){
                match = compare(qualityOp, height, qualityFilter);
            }
        }
    }
    return match;
}
bool SceneProxyModel::filterMatches_rating(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(ratingOp)){
        match = false;
        QString r = getCellData(row, SCENE_RATING_COLUMN, index);
        if (!r.isEmpty()){
            Rating rating(r);
            match = compare(ratingOp, rating, ratingFilter);
        }
    }
    return match;
}
bool SceneProxyModel::filterMatches_filesize(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(sizeOp) && (sizeFilter > 0)){
        match = false;
        QString s = getCellData(row, SCENE_SIZE_COLUMN, index);
        if (!s.isEmpty()){
            bool ok = false;
            if (s.contains("GB")){
                s = s.remove(QRegularExpression("\\s*GB\\s*")).trimmed();
                double d = s.toDouble(&ok);
                if (ok){
                    match = compare(sizeOp, (qint64)(d * 1024), sizeFilter);
                }
            } else {
                s = s.remove(QRegularExpression("\\s*MB\\s*")).trimmed();
                qint64 i = (qint64)(s.toInt(&ok));
                if (ok){
                    match = compare(sizeOp, i, sizeFilter);
                }
            }
        }
    }
    return match;
}

bool SceneProxyModel::filterMatches_duration(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(durationOp) && (durationFilter.isValid())){
        match = false;
        QString s = getCellData(row, SCENE_LENGTH_COLUMN, index);
        if (!s.isEmpty()){
            QTime t = QTime::fromString(s, "h:mm:ss");
            if (t.isValid()){
                match = compare(durationOp, t, durationFilter);
            }
        }
    }
    return match;
}

bool SceneProxyModel::filterMatches_release(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(releasedOp) && (releasedFilter.isValid())){
        match = false;
        QString s = getCellData(row, SCENE_DATE_COLUMN, index);
        if (!s.isEmpty()){
            QDate d = QDate::fromString(s, "yyyy/MM/dd");
            if (d.isValid()){
                match = compare(releasedOp, d, releasedFilter);
            }
        }
    }
    return match;
}

bool SceneProxyModel::filterMatches_tag(int /*row*/, const QModelIndex &/*index*/) const{
    QString tag = "";
    QRegularExpression rx(tagFilter);
    QRegularExpressionMatch match = rx.match(tag);
    return match.hasMatch();
}

bool SceneProxyModel::filterMatches_company(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(companyFilter)){
        match = false;
        QString company = getCellData(row, SCENE_COMPANY_COLUMN, index);
        if (!company.isEmpty()){
            QRegularExpression rx(companyFilter);
            QRegularExpressionMatch m = rx.match(company);
            match = m.hasMatch();
        }
    }
    return match;
}
