#include "FilterSet.h"
#include <QPair>
#include "SceneProxyModel.h"
#include "genericfunctions.h"
#include "config.h"
#include <QTextStream>
FilterSet::FilterSet(){
    clear();
}

FilterSet::FilterSet(const SceneProxyModel *s){
    this->fileFilter        = s->filterFilename();
    this->nameFilter        = s->filterActor();
    this->tagFilter         = s->filterTag();
    this->companyFilter     = s->filterCompany();
    QPair<LogicalOperator, int>    filterQuality = s->filterQuality();
    this->qualityOp         = filterQuality.first;
    this->qualityFilter     = filterQuality.second;
    QPair<LogicalOperator, QTime>  filterDuration = s->filterDuration();
    this->durationOp        = filterDuration.first;
    this->durationFilter    = filterDuration.second;
    QPair<LogicalOperator, Rating> filterRating = s->filterRating();
    this->ratingOp          = filterRating.first;
    this->ratingFilter      = filterRating.second;
    QPair<LogicalOperator, qint64> filterSize = s->filterSize();
    this->sizeOp            = filterSize.first;
    this->sizeFilter        = filterSize.second;
    QPair<LogicalOperator, QDate>  filterReleased = s->filterReleased();
    this->releasedOp        = filterReleased.first;
    this->releasedFilter    = filterReleased.second;
    QPair<LogicalOperator, QDate>  filterAdded = s->filterAdded();
    this->addedOp           = filterAdded.first;
    this->addedFilter       = filterAdded.second;
}

void FilterSet::clear(){
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
}
QString FilterSet::writeLine(QString key, QString value){
    return QString("%1  =   %2\n").arg(key).arg(value);
}

bool FilterSet::save(const QString &filepath){
    bool success = false;
    QFile file(filepath);
    if (!file.open(QFile::Text | QFile::WriteOnly)){
        qWarning("Error: Unable to open %s for writing", qPrintable(filepath));
    } else {
        QTextStream out(&file);
        out << writeLine(FILTER_DURATION_OP, toString(durationOp));
        out << writeLine(FILTER_QUALITY_OP, toString(qualityOp));
        out << writeLine(FILTER_RATING_OP, toString(ratingOp));
        out << writeLine(FILTER_SIZE_OP, toString(sizeOp));
        out << writeLine(FILTER_RELEASE_OP, toString(releasedOp));
        out << writeLine(FILTER_ADDED_OP, toString(addedOp));
        out << writeLine(FILTER_ACTOR, nameFilter);
        out << writeLine(FILTER_TAG, tagFilter);
        out << writeLine(FILTER_WORD, fileFilter);
        out << writeLine(FILTER_COMPANY, companyFilter);
        out << writeLine(FILTER_SIZE, QString::number(sizeFilter));
        out << writeLine(FILTER_QUALITY, QString::number(qualityFilter));
        out << writeLine(FILTER_DURATION, durationFilter.toString("h:mm:ss"));
        out << writeLine(FILTER_RELEASE, releasedFilter.toString("yyyy-MM-dd"));
        out << writeLine(FILTER_ADDED, addedFilter.toString("yyyy-MM-dd"));
        out << writeLine(FILTER_RATING, ratingFilter.grade());
        success = file.close();
        qDebug("Filters saved to '%s'", qPrintable(filepath));
    }
    return success;
}

