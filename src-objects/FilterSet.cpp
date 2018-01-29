#include "FilterSet.h"
#include <QPair>
#include "ActorProxyModel.h"
#include "SceneProxyModel.h"
#include "genericfunctions.h"
#include "config.h"
#include <QTextStream>
FilterSet::FilterSet(){
    clear();
}

FilterSet::FilterSet(const SceneProxyModel *s):
    filterType(SCENE_FILTER){
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
FilterSet::FilterSet(const ActorProxyModel *a):
    filterType(ACTOR_FILTER){
    nameFilter = a->getFilterName();
    hairFilter = a->getFilterHair();
    skinFilter = a->getFilterEthnicity();
    tattooFilter = a->getFilterTattoos();
    piercingFilter = a->getFilterPiercings();
    QPair<LogicalOperator, Height> height = a->getFilterHeight();
    heightOp         = height.first;
    heightFilter     = height.second;
    QPair<LogicalOperator, int> weight = a->getFilterWeight();
    weightOp         = weight.first;
    weightFilter     = weight.second;
    QPair<LogicalOperator, int> count = a->getFilterSceneCount();
    sceneCountOp     = count.first;
    sceneCountFilter = count.second;
}

void FilterSet::clear(){
    /// Scene Filters
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
    /// Actor Filters
    hairFilter = "";
    skinFilter = "";
    heightFilter = Height();
    tattooFilter = DONT_CARE;
    piercingFilter = DONT_CARE;
    sceneCountFilter = -1;
    ageFilter = -1;
    weightFilter = -1;
    sceneCountOp = NOT_SET;
    heightOp = NOT_SET;
    weightOp = NOT_SET;
    ageOp = NOT_SET;
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
        if (filterType == SCENE_FILTER){
            out << writeLine(FILTER_TYPE,       FILTER_TYPE_SCENE);
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
        } else {
            out << writeLine(FILTER_TYPE,   FILTER_TYPE_ACTOR);
            out << writeLine(FILTER_ACTOR,   nameFilter);
            out << writeLine(FILTER_ETHNICITY, skinFilter);
            out << writeLine(FILTER_HEIGHT, heightFilter.toString());
            out << writeLine(FILTER_HEIGHT_OP, toString(heightOp));
            out << writeLine(FILTER_WEIGHT, QString::number(weightFilter));
            out << writeLine(FILTER_WEIGHT_OP, toString(weightOp));
            out << writeLine(FILTER_AGE,    QString::number(ageFilter));
            out << writeLine(FILTER_AGE_OP, toString(ageOp));
            out << writeLine(FILTER_SCENE_COUNT, QString::number(sceneCountFilter));
            out << writeLine(FILTER_SCENE_COUNT_OP, toString(sceneCountOp));
            out << writeLine(FILTER_TATTOOS, toString(tattooFilter));
            out << writeLine(FILTER_PIERCINGS, toString(piercingFilter));
        }
        file.close();
        success = true;
        qDebug("Filters saved to '%s'", qPrintable(filepath));
    }
    return success;
}

void FilterSet::read(const QString filename){
    QFile file(filename);
    if (!file.exists() || !file.open(QFile::Text | QFile::ReadOnly)){
        qCritical("Error Opening %s for reading in as a filter set", qPrintable(filename));
        return;
    }
    QTextStream in(&file);
    while(!in.atEnd()){
        readLine(in.readLine());
    }
}

bool FilterSet::readLine(const QString &s){
    QRegularExpression rx("([A-Z_]+)=(.*)");
    QRegularExpressionMatch m = rx.match(s);
    if (!m.hasMatch()){
        return false;
    } else {
        QString k = m.captured(1);
        QString v = m.captured(2);
        if (k == FILTER_TYPE){
            if (v == FILTER_TYPE_ACTOR){
                this->filterType = ACTOR_FILTER;
            } else {
                this->filterType = SCENE_FILTER;
            }
        } else if (k == FILTER_ACTOR){
            this->nameFilter = v;
        } else if (k == FILTER_ETHNICITY){
            skinFilter = v;
        } else if (k == FILTER_HAIR){
            hairFilter = v;
        } else if (k == FILTER_HEIGHT){
            heightFilter = Height(v);
        } else if (k == FILTER_HEIGHT_OP){
            heightOp = fromString(v);
        } else if (k == FILTER_WEIGHT){
            weightFilter = v.toInt();
        } else if (k == FILTER_WEIGHT_OP) {
            weightOp = fromString(v);
        } else if (k == FILTER_SCENE_COUNT){
            sceneCountFilter = v.toInt();
        } else if (k == FILTER_SCENE_COUNT_OP){
            sceneCountOp = fromString(v);
        } else if (k == FILTER_TATTOOS){
            tattooFilter = triStateFromString(v);
        } else if (k == FILTER_PIERCINGS) {
            piercingFilter = triStateFromString(v);
        } else if (k == FILTER_ADDED){
            addedFilter = QDate::fromString("yyyy-MM-dd");
        } else if (k == FILTER_ADDED_OP){
            addedOp = fromString(v);
        } else if (k == FILTER_RELEASE){
            releasedFilter = QDate::fromString("yyyy-MM-dd");
        } else if (k == FILTER_RELEASE_OP){
            releasedOp = fromString(v);
        } else if (k == FILTER_DURATION){
            durationFilter = QTime::fromString("h:mm:ss");
        } else if (k == FILTER_DURATION_OP) {
            durationOp = fromString(v);
        } else if (k == FILTER_COMPANY){
            companyFilter = v;
        } else if (k == FILTER_QUALITY) {
            qualityFilter = v.toInt();
        } else if (k == FILTER_QUALITY_OP) {
            qualityOp = fromString(v);
        } else if (k == FILTER_RATING) {
            ratingFilter = Rating(v);
        } else if (k == FILTER_RATING_OP){
            ratingOp = fromString(v);
        } else if (k == FILTER_SERIES){
            seriesFilter = v;
        } else if (k == FILTER_TITLE){
            titleFilter = v;
        } else if (k == FILTER_SIZE){
            sizeFilter = v.toInt();
        } else if (k == FILTER_SIZE_OP){
            sizeOp = fromString(v);
        } else if (k == FILTER_TAG) {
            tagFilter = v;
        } else if (FILTER_WORD){
            fileFilter = v;
        }
    }
    return true;
}

