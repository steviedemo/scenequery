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
    nameFilter      = ".*";
    companyFilter   = ".*";
    fileFilter      = ".*";
    tagFilter       = ".*";
    gradeFilter     = ".*";
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
    qDebug("Setting Scene Filter to '%s'", qPrintable(text));
    invalidateFilter();
    this->setFilterRegExp(text);
    this->setFilterFixedString(text);
}

bool SceneProxyModel::filterMatchesTriState(const TriState &t, const QString &s) const {
    bool result = (!s.isEmpty() && (s != "None"));
    bool filterMatches = false;
    if (t == ON ){
        filterMatches = result;
    } else if (t == OFF){
        filterMatches = (!result);
    } else {
        filterMatches = true;
    }
    return filterMatches;
}
bool SceneProxyModel::filterMatchesAnything(const QString &s) const {
    bool match = (s.isEmpty() || (s == ".*")  || (s == ".*.*") || (s == ".*.*.*") || s.contains("No Selection"));
    return match;
}
QString SceneProxyModel::getCellData(int row, int column, const QModelIndex &sourceParent) const{
    return sourceModel()->data(sourceModel()->index(row, column, sourceParent)).toString();
}

bool SceneProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const{
/*
    QString criteria("");
    QTextStream out(&criteria);
*/
    bool fileMatch(true), nameMatch(true),durationMatch(true),companyMatch(true),sizeMatch(true),heightMatch(true),ratingMatch(true),releaseMatch(true);
    if (!filterMatchesAnything(fileFilter)){
    //    out << " | File Filter: '" << fileFilter << "'";
        fileMatch = filterMatches_filename  (source_row, source_parent);
    }
    if (!filterMatchesAnything(nameFilter)){
    //    out << " | Name Filter: '" << nameFilter << "'";
        nameMatch = filterMatches_name(source_row, source_parent);
    }
    if (!filterMatchesAnything(companyFilter)){
    //    out << " | Company Filter: '" << companyFilter << "'";
        companyMatch = filterMatches_company(source_row, source_parent);
    }
    if (!filterMatchesAnything(durationFilter) && !filterMatchesAnything(durationOp)){
    //    out << " | Length Filter: " << toString(durationOp) << " " << durationFilter.toString("h:mm:ss");
        durationMatch = filterMatches_duration(source_row, source_parent);
    }
    if (!filterMatchesAnything(sizeOp) && !filterMatchesAnything(sizeFilter)){
    //    out << " | Size Filter: " << toString(sizeOp) << " " << sizeFilter;
        sizeMatch = filterMatches_filesize  (source_row, source_parent);
    }
    if (!filterMatchesAnything(qualityOp) && !filterMatchesAnything(qualityFilter)){
    //    out << " | Height Filter: " << toString(qualityOp) << " " << qualityFilter;
        heightMatch = ((qualityFilter == -1)      || filterMatches_quality   (source_row, source_parent));
    }
    if (!filterMatchesAnything(ratingFilter) && !filterMatchesAnything(ratingOp)){
    //    out << " | Rating Filter: " << toString(ratingOp) << " " << ratingFilter.grade();
        ratingMatch = ((!ratingFilter.isValid())  || filterMatches_rating    (source_row, source_parent));
    }
    if (!filterMatchesAnything(releasedOp) && !filterMatchesAnything(releasedFilter)){
    //    out << " | Release Date Filter: " << toString(releasedOp) << " " << releasedFilter.toString("yyyy.MM.dd");
        releaseMatch = (!releasedFilter.isValid()  || filterMatches_release   (source_row, source_parent));
    }
    bool match = (fileMatch && nameMatch && durationMatch && companyMatch && sizeMatch && heightMatch && ratingMatch && releaseMatch);
    /*
    if (match && !criteria.isEmpty()){
        QString filename = getCellData(source_row, SCENE_PATH_COLUMN, source_parent);
        filename.remove(QRegularExpression(".+\\/"));    
    }
    */
    return match;
}

void SceneProxyModel::loadFilters(FilterSet set){
    QPair<LogicalOperator, QTime> filterDuration= set.getFilterDuration();
    this->durationOp = filterDuration.first;
    this->durationFilter = filterDuration.second;
    QPair<LogicalOperator, QDate> filterRelease = set.getFilterRelease();
    this->releasedOp = filterRelease.first;
    this->releasedFilter = filterRelease.second;
    QPair<LogicalOperator, QDate> filterAdded   = set.getFilterAdded();
    this->addedOp = filterAdded.first;
    this->addedFilter = filterAdded.second;
    QPair<LogicalOperator, int>   filterQuality = set.getFilterQuality();
    this->qualityOp = filterQuality.first;
    this->qualityFilter = filterQuality.second;
    QPair<LogicalOperator, qint64>filterSize    = set.getFilterSize();
    this->sizeOp = filterSize.first;
    this->sizeFilter = filterSize.second;
    QPair<LogicalOperator, Rating>filterRating  = set.getFilterRating();
    this->ratingOp = filterRating.first;
    this->ratingFilter = filterRating.second;
    this->idFilter      = set.getFilterID();
    this->titleFilter   = set.getFilterTitle();
    this->nameFilter    = set.getFilterName();
    this->companyFilter = set.getFilterCompany();
    this->tagFilter     = set.getFilterTags();
    this->fileFilter    = set.getFilterFilename();
    this->seriesFilter  = set.getFilterSeries();
}

bool SceneProxyModel::filterMatches_filename(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(fileFilter)){
        QRegularExpression rx(fileFilter);
        QString filenameData = getCellData(row, SCENE_PATH_COLUMN, index);
        match = rx.match(filenameData).hasMatch();
    }
    return match;
}

bool SceneProxyModel::filterMatches_name(int row, const QModelIndex &currIndex) const{
    bool match = true;
    if (!filterMatchesAnything(nameFilter)){
        QString mainName = getCellData(row, SCENE_NAME_COLUMN, currIndex);
        QString featNames = getCellData(row, SCENE_FEATURED_COLUMN, currIndex);
        QRegularExpression rx(nameFilter);
        match = (rx.match(mainName).hasMatch() || rx.match(featNames).hasMatch());
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
    if (!filterMatchesAnything(ratingOp) && !ratingFilter.isEmpty()){
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
    if (!filterMatchesAnything(durationOp) && durationFilter.isValid()){
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

bool SceneProxyModel::filterMatches_title(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(titleFilter)){
        QString title = getCellData(row, SCENE_TITLE_COLUMN, index);
        QRegularExpression rx(titleFilter);
        match = rx.match(title).hasMatch();
    }
    return match;
}

bool SceneProxyModel::filterMatches_tag(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(tagFilter)){
        QString tags = getCellData(row, SCENE_TAG_COLUMN, index);
        QRegularExpression rx(tagFilter);
        match = rx.match(tags).hasMatch();
    }
    return match;
}

bool SceneProxyModel::filterMatches_company(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(companyFilter)){
        QRegularExpression rx(companyFilter);
        QString company = getCellData(row, SCENE_COMPANY_COLUMN, index);
        match = rx.match(company).hasMatch();
    }
    return match;
}

bool SceneProxyModel::filterMatches_series(int row, const QModelIndex &index) const{
    bool match = true;
    if (!filterMatchesAnything(seriesFilter)){
        QRegularExpression rx(seriesFilter);
        QString data = getCellData(row, SCENE_SERIES_COLUMN, index);
        match = rx.match(data).hasMatch();
    }
    return match;
}
