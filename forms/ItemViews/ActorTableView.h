#ifndef ACTORTABLEVIEW_H
#define ACTORTABLEVIEW_H
#include <QtWidgets>
#include <QWidget>
#include "ActorProxyModel.h"
#include "DataManager.h"
#include "FilterSet.h"
#include "definitions.h"
#include <QFrame>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include "SceneTableView.h"
#include "SceneDetailView.h"
#include "ActorProfileView.h"
#include "FilterSet.h"
class ActorTableView : public QWidget
{
    Q_OBJECT
public:
    ActorTableView                  (QWidget *parent =0);
    void setSourceModel             (QAbstractItemModel *model);
    void setDataContainers          (QSharedPointer<DataManager> vault)   {   this->vault = vault;            }
    void connectViews               (SceneTableView *, SceneDetailView *, ActorProfileView *);
    void setHorizontalHeaders       (QStringList);
    int countRows                   (void);
    QStringList namesDisplayed      (void);
    QString selectedName            (void) const;
    void resizeToContents           (void);
    QModelIndex currentIndex        (void);
    QModelIndex findActorIndex      (const QString &name) const;
    QModelIndex findActorIndex_Exact(const QString &name) const;
    QModelIndex findActorIndex_base (const QRegExp &name, const int column) const;
public slots:
    void loadFilters        (FilterSet filters);
    FilterSet saveFilters   (void);
    void addRows            (RowList rows);
    void addActor           (ActorPtr);
    void addNewActors       (const ActorList &list);
    void addNewActor        (const ActorPtr a);
    void clearFilters       (void);
    void resizeView         (void);
    void setFilter_name     (const QString name="") {   proxyModel->setFilterName(".*"+name+".*");        }
    void setFilter_hair     (const QString hair="") {   proxyModel->setFilterHairColor(".*"+hair+".*");   }
    void setFilter_ethnicity(const QString skin="") {   proxyModel->setFilterEthnicity(".*"+skin+".*");   }
    void setFilter_age      (const int age=-1,             const LogicalOperator op=NOT_SET)    {   proxyModel->setFilterAge(age, op);          }
    void setFilter_height   (const Height height=Height(), const LogicalOperator op=NOT_SET)    {   proxyModel->setFilterHeight(height, op);    }
    void setFilter_weight   (const int weight=-1,          const LogicalOperator op=NOT_SET)    {   proxyModel->setFilterWeight(weight, op);    }
    void setFilter_tattoos  (const TriState tattoos=DONT_CARE)  {   proxyModel->setFilterTattoos(tattoos);  }
    void setFilter_piercings(const TriState rings=DONT_CARE)    {   proxyModel->setFilterPiercings(rings);  }

    void filterChanged(QString);
    void filterChangedName(const QString name="");
    void filterChangedHair(QString filter="");
    void filterChangedEthnicity(QString filter="");
    void filterChangedSceneCount(ActorProxyModel::NumberFilterType, int);
    void selectActor(QString name);
    void selectActor(ActorPtr actor);
    void removeActor(QString name);
    void removeActor(ActorPtr);
private:
    QWidget *parent;
    QString ethnicityFilter, nameFilter, hairFilter, currentSelection;
    QStandardItem *actorParent;
    QStandardItemModel *actorModel;
    QSharedPointer<DataManager> vault;
    ActorProxyModel *proxyModel;
    QModelIndex currentIdx, prevIdx;
    QTableView *table;
    QVBoxLayout *mainLayout;
    QItemSelectionModel *selectionModel;
    SceneTableView *sceneTableView;
    SceneDetailView *detailView;
    ActorProfileView *profileView;
    bool itemClicked;
    void addDeleteButtons();
private slots:
    void selectionChanged(QModelIndex, QModelIndex);
    void rowCountChanged(QModelIndex, int, int);
    void rowClicked(QModelIndex);
signals:
    void rowsFinishedLoading();
    void displayChanged(int);
    void actorClicked(QString);
    void actorSelectionChanged(QString name);
    void progressBegin(QString, int);
    void progressUpdate(int);
    void progressEnd(QString);
    void deleteActor(QString);
    void saveFilterSet(FilterSet);
};

#endif // ACTORTABLEVIEW_H
