#ifndef ACTORPROXYMODEL_H
#define ACTORPROXYMODEL_H
#include "definitions.h"
#include "Height.h"
#include <QSortFilterProxyModel>
#include <QPair>
#include <QRegularExpression>
class ActorProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum NumberFilterType{ greater_than, less_than, equal_to };
    ActorProxyModel(QObject *parent = 0);
    void        clearFilters();
    QString     getFilterHair()         const   {   return hairFilter;      }
    QString     getFilterEthnicity()    const   {   return skinFilter;      }
    int         getFilterSceneCount()   const   {   return sceneCountFilter;}
    TriState    getFilterTattoos()      const   {   return tattooFilter;    }
    TriState    getFilterPiercings()    const   {   return piercingFilter;  }
    QPair<LogicalOperator,Height> getFilterHeight()     const { return QPair<LogicalOperator,Height>(heightOp, heightFilter);   }
    QPair<LogicalOperator,int>    getFilterWeight()     const { return QPair<LogicalOperator,int>   (weightOp, weightFilter);   }
    QPair<LogicalOperator,int>    getFilterSceneCount() const { return QPair<LogicalOperator,int>   (countOp, sceneCountFilter);}
public slots:
    void setFilterName          (const QString name="")      {   nameFilter = QString(".*%1.*").arg(name);       }
    void setFilterHairColor     (const QString hairColor="") {   hairFilter = QString(".*%1.*").arg(hairColor);  }
    void setFilterEthnicity     (const QString ethnicity="") {   skinFilter = QString(".*%1.*").arg(ethnicity);  }
    void setFilterSceneCount    (const int count=-1, LogicalOperator op=NOT_SET){   countOp = op; sceneCountFilter = count; }
    void setFilterHeight        (const Height h,     LogicalOperator op=NOT_SET){   heightOp =op; heightFilter = h;         }
    void setFilterWeight        (const int weight=-1,LogicalOperator op=NOT_SET){   weightOp =op; weightFilter = weight;    }
    void setFilterAge           (const int age=-1,   LogicalOperator op=NOT_SET){   ageOp    =op; ageFilter = age;          }
    void setFilterTattoos       (const TriState tattoos = NOT_SET)    {   tattooFilter    = tattoos;      }
    void setFilterPiercings     (const TriState piercings = NOT_SET)  {   piercingFilter  = piercings;    }
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
private:
    QRegularExpression hairRx, skinRx, anythingRx, nameRx;
    QRegularExpressionMatch match;
    QString nameFilter, hairFilter, skinFilter;
    Height heightFilter;
    TriState tattooFilter, piercingFilter;
    int sceneCountFilter, ageFilter, weightFilter;
    LogicalOperator countOp, heightOp, weightOp, ageOp;
    QString getCellData(int row, int col, const QModelIndex &currIndex) const;
    bool filterMatches_name     (int row, const QModelIndex &index) const;
    bool filterMatches_hair     (int row, const QModelIndex &index) const;
    bool filterMatches_skin     (int row, const QModelIndex &index) const;
    bool filterMatches_height   (int row, const QModelIndex &index) const;
    bool filterMatches_weight   (int row, const QModelIndex &index) const;
    bool filterMatches_tattoos  (int row, const QModelIndex &index) const;
    bool filterMatches_piercings(int row, const QModelIndex &index) const;
    bool filterMatches_scenes   (int row, const QModelIndex &index) const;
    bool filterMatches_age      (int row, const QModelIndex &index) const;
};

#endif // ACTORPROXYMODEL_H
