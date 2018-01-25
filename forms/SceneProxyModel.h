#ifndef SCENEPROXYMODEL_H
#define SCENEPROXYMODEL_H
#include <QDate>
#include <QTime>
#include <QPair>
#include <QSortFilterProxyModel>
#include "definitions.h"
#include "FilterSet.h"
#include "Rating.h"
class SceneProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SceneProxyModel(QObject *parent = 0);
    void    clearFilters        (void);
    void    setFilterActor      (const QString name=".*")                           {   this->nameFilter = name;        setFilter(name);         }
    void    setFilterCompany    (const QString company=".*")                        {   this->companyFilter = company;  setFilter(company);      }
    void    setFilterTag        (const QString tag=".*")                            {   this->tagFilter = tag;          setFilter(tag);          }
    void    setFilterFilename   (const QString searchTerm=".*")                     {   this->fileFilter = searchTerm;  setFilter(searchTerm);   }
    void    setFilterID         (const int id       = -1)                           {   this->idFilter = id;            }
    void    setFilterQuality    (const int quality  = -1,       const LogicalOperator op=NOT_SET) {   this->qualityOp = op;   this->qualityFilter = quality;}
    void    setFilterDuration   (const QTime time   = QTime(),  const LogicalOperator op=NOT_SET) {   this->durationOp = op;  this->durationFilter = time;  }
    void    setFilterRating     (const Rating rating= Rating(), const LogicalOperator op=NOT_SET) {   this->ratingOp = op;    this->ratingFilter = rating;  }
    void    setFilterSize       (const qint64 size  = -1,       const LogicalOperator op=NOT_SET) {   this->sizeOp = op;      this->sizeFilter = size;      }
    void    setFilterRelease    (const QDate date   = QDate(),  const LogicalOperator op=NOT_SET) {   this->releasedOp= op;   this->releasedFilter = date;  }
    void    setFilterAdded      (const QDate date   = QDate(),  const LogicalOperator op=NOT_SET) {   this->addedOp = op;     this->addedFilter = date;     }
    QString filterActor()                           const                           {   return this->nameFilter;                          }
    QString filterCompany()                         const                           {   return this->companyFilter;                       }
    QString filterTag()                             const                           {   return this->tagFilter;                           }
    int     filterID()                              const                           {   return this->idFilter;                            }
    QString filterFilename()                        const                           {   return this->fileFilter;                          }
    QPair<LogicalOperator, int>    filterQuality()  const                           {   return QPair<LogicalOperator, int>      (qualityOp,  qualityFilter);    }
    QPair<LogicalOperator, QTime>  filterDuration() const                           {   return QPair<LogicalOperator, QTime>    (durationOp, durationFilter);   }
    QPair<LogicalOperator, Rating> filterRating()   const                           {   return QPair<LogicalOperator, Rating>   (ratingOp,   ratingFilter);     }
    QPair<LogicalOperator, qint64> filterSize()     const                           {   return QPair<LogicalOperator, qint64>   (sizeOp,     sizeFilter);       }
    QPair<LogicalOperator, QDate>  filterReleased() const                           {   return QPair<LogicalOperator, QDate>    (releasedOp, releasedFilter);   }
    QPair<LogicalOperator, QDate>  filterAdded()    const                           {   return QPair<LogicalOperator, QDate>    (addedOp,    addedFilter);      }
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
private:
    bool filterMatchesAnything(const QString &s) const;
    bool filterMatchesAnything(const LogicalOperator &op) const;
    void setFilter(QString text);
    QString getCellData(int row, int column, const QModelIndex &sourceParent) const;
    bool filterMatches_filename (int row, const QModelIndex & index) const;
    bool filterMatches_name     (int row, const QModelIndex &index) const;
    bool filterMatches_company  (int row, const QModelIndex &index) const;
    bool filterMatches_tag      (int row, const QModelIndex &index) const;
    bool filterMatches_quality  (int row, const QModelIndex &index) const;
    bool filterMatches_duration (int row, const QModelIndex &index) const;
    bool filterMatches_filesize (int row, const QModelIndex &index) const;
    bool filterMatches_rating   (int row, const QModelIndex &index) const;
    bool filterMatches_added    (int row, const QModelIndex &index) const;
    bool filterMatches_release  (int row, const QModelIndex &index) const;
    LogicalOperator durationOp, qualityOp, sizeOp, ratingOp, releasedOp, addedOp;
    QString nameFilter, companyFilter, tagFilter, fileFilter;
    qint64 sizeFilter;
    QTime durationFilter;
    QDate addedFilter, releasedFilter;
    Rating ratingFilter;
    QString gradeFilter;
    int qualityFilter, idFilter;
};

#endif // SCENEPROXYMODEL_H
