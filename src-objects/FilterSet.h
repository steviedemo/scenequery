#ifndef FILTERSET_H
#define FILTERSET_H
#include "Height.h"
#include "Rating.h"
#include <QDate>
#include <QTime>
#include <QPair>
#include "definitions.h"


#define FILTER_DURATION     "DURATION"
#define FILTER_DURATION_OP  "DURATION_OP"
#define FILTER_QUALITY      "QUALITY"
#define FILTER_QUALITY_OP   "QUALITY_OP"
#define FILTER_RATING       "RATING"
#define FILTER_RATING_OP    "RATING_OP"
#define FILTER_RELEASE      "RELEASE"
#define FILTER_RELEASE_OP   "RELEASE_OP"
#define FILTER_ADDED        "ADDED"
#define FILTER_ADDED_OP     "ADDED_OP"
#define FILTER_SIZE         "SIZE"
#define FILTER_SIZE_OP      "SIZE_OP"
#define FILTER_COMPANY      "COMPANY"
#define FILTER_SERIES       "SERIES"
#define FILTER_TITLE        "TITLE"
#define FILTER_ACTOR        "ACTOR"
#define FILTER_WORD         "WORD"
#define FILTER_TAG          "TAG"

#define FILTER_HAIR         "HAIR_COLOR"
#define FILTER_ETHNICITY    "ETHNICITY"
#define FILTER_HEIGHT       "HEIGHT"
#define FILTER_HEIGHT_OP    "HEIGHT_OP"
#define FILTER_SCENE_COUNT  "SCENE_COUNT"
#define FILTER_SCENE_COUNT_OP "SCENE_COUNT_OP"
#define FILTER_AGE          "AGE"
#define FILTER_AGE_OP       "AGE_OP"
#define FILTER_WEIGHT       "WEIGHT"
#define FILTER_WEIGHT_OP    "WEIGHT_OP"
#define FILTER_TATTOOS      "TATTOOS"
#define FILTER_PIERCINGS    "PIERCINGS"
#define FILTER_TYPE_SCENE   "SCENE_TYPE"
#define FILTER_TYPE_ACTOR   "ACTOR_TYPE"
#define FILTER_TYPE         "FILTER_TYPE"

enum  FilterType{ ACTOR_FILTER, SCENE_FILTER };
class FilterSet
{
public:
    FilterSet();
    explicit FilterSet(const class SceneProxyModel *);
    explicit FilterSet(const class ActorProxyModel *);
    void clear();
    bool save(const QString &filepath);
    void read(const QString filepath);
    FilterType getFilterType()  const { return filterType;      }
    /// Scene Filters
    QPair<LogicalOperator, QTime> getFilterDuration() const { return QPair<LogicalOperator, QTime>(durationOp, durationFilter); }
    QPair<LogicalOperator, QDate> getFilterRelease()  const { return QPair<LogicalOperator, QDate>(releasedOp, releasedFilter); }
    QPair<LogicalOperator, QDate> getFilterAdded()    const { return QPair<LogicalOperator, QDate>(addedOp, addedFilter);       }
    QPair<LogicalOperator, int>   getFilterQuality()  const { return QPair<LogicalOperator, int>  (qualityOp, qualityFilter);   }
    QPair<LogicalOperator, qint64>getFilterSize()     const { return QPair<LogicalOperator, qint64>(sizeOp, sizeFilter);        }
    QPair<LogicalOperator, Rating>getFilterRating()   const { return QPair<LogicalOperator, Rating>(ratingOp, ratingFilter);    }
    int     getFilterID()       const { return idFilter;        }
    QString getFilterTitle()    const { return titleFilter;     }
    QString getFilterName()     const { return nameFilter;      }
    QString getFilterCompany()  const { return companyFilter;   }
    QString getFilterTags()     const { return tagFilter;       }
    QString getFilterFilename() const { return fileFilter;      }
    QString getFilterSeries()   const { return seriesFilter;    }
    /// Actor Filters
    QString getFilterEthnicity()const{ return skinFilter;       }
    TriState getFilterTattoos() const{ return tattooFilter;     }
    TriState getFilterPiercings() const { return piercingFilter;    }
    QPair<LogicalOperator, Height> getFilterHeight() const { return QPair<LogicalOperator, Height>(heightOp, heightFilter);   }
    QPair<LogicalOperator, int>    getFilterSceneCount() const { return QPair<LogicalOperator, int>(sceneCountOp, sceneCountFilter);    }
    QPair<LogicalOperator, int>    getFilterWeight() const { return QPair<LogicalOperator, int>(weightOp, weightFilter);    }
    QPair<LogicalOperator, int>    getFilterAge() const { return QPair<LogicalOperator, int>(ageOp, ageFilter); }

private:
    FilterType filterType;
    QString writeLine(QString, QString);
    bool readLine(const QString &s);
    /// Scene Filters
    LogicalOperator durationOp, qualityOp, sizeOp, ratingOp, releasedOp, addedOp;
    QString nameFilter, companyFilter, tagFilter, fileFilter, titleFilter, seriesFilter;
    qint64 sizeFilter;
    QTime durationFilter;
    QDate addedFilter, releasedFilter;
    Rating ratingFilter;
    int qualityFilter, idFilter;
    /// ActorFilters
    QString hairFilter, skinFilter;
    Height heightFilter;
    TriState tattooFilter, piercingFilter;
    int sceneCountFilter, ageFilter, weightFilter;
    LogicalOperator sceneCountOp, heightOp, weightOp, ageOp;
};

#endif // FILTERSET_H
