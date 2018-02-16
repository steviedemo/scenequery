#ifndef SCENEVIEW_H
#define SCENEVIEW_H
#include <QtWidgets>
#include <QWidget>
#include <QStandardItemModel>
#include <QSharedPointer>
#include "ActorProfileView.h"
#include "DataManager.h"
#include "definitions.h"
#include "FilterSet.h"
#include "SceneProxyModel.h"
#include "SceneList.h"
#include "SceneDetailView.h"
class SceneTableView : public QWidget
{
    Q_OBJECT
public:
    explicit SceneTableView(QWidget *parent = 0);
    void setSourceModel(QAbstractItemModel *model);
    QVector<int> getIDs();
    void updateSceneDisplay(int);
    int countRows(void) const { return proxyModel->rowCount();  }
    void setDataContainers(QSharedPointer<DataManager> vault)   {  this->vault = vault;         }
    void connectViews(SceneDetailView *, ActorProfileView *);
public slots:
    void loadFilters(FilterSet set);
    void saveFilters(void);
    void addRow(const Row);
    void addRows(RowList);
    void addNewScene(ScenePtr);
    void addNewScenes(SceneList);
    void receiveSceneCountRequest();
    void resizeSceneView()          {   table->resizeColumnsToContents(); table->resizeRowsToContents();    }
    void updateSceneItem(int id);
    void purgeSceneItems(QVector<int>);
    void setFilter_name     (const ActorPtr a);
    void setFilter_name     (const QString name=".*")       { proxyModel->setFilterActor    (".*"+name+".*");       }
    void setFilter_company  (const QString company=".*")    { proxyModel->setFilterCompany  (".*"+company+".*");    }
    void setFilter_tags     (const QString tags=".*")       { proxyModel->setFilterTag      (".*"+tags+".*");       }
    void setFilter_filename (const QString word=".*")       { proxyModel->setFilterFilename (".*"+word+".*");       }
    void setFilter_duration (const QTime t=QTime(), const LogicalOperator op=NOT_SET) { proxyModel->setFilterDuration(t, op);   }
    void setFilter_quality  (const int q=-1, const LogicalOperator op=NOT_SET)        { proxyModel->setFilterQuality(q, op);    }
    void setFilter_id       (const int id=-1)   {   proxyModel->setFilterID(id);    }


    void actorFilterChanged(const QString s)            {   proxyModel->setFilterActor(   QString(".*%1.*").arg(s)); }
    void companyFilterChanged(QString s="")             {   proxyModel->setFilterCompany( QString(".*%1.*").arg(s)); }
    void tagFilterChanged(QString s="")                 {   proxyModel->setFilterTag(     QString(".*%1.*").arg(s)); }
    void filenameFilterChanged(const QString s="")      {   proxyModel->setFilterFilename(QString(".*%1.*").arg(s));    }
    void qualityFilterChanged   (const int i=-1,  const LogicalOperator op=NOT_SET) {  proxyModel->setFilterQuality(i, op);    }
    void durationFilterChanged  (QTime t=QTime(), const LogicalOperator op=NOT_SET) {  proxyModel->setFilterDuration(t, op);   }
    void searchByID(const int &);
    QVector<int> getSelectedIDs(void) const;

private slots:
    void rowCountChanged(QModelIndex, int, int);
    void clearFilters(void)      {   proxyModel->clearFilters(); nameFilter = ".*"; fileFilter = ".*";         }
    void clearActorFilterOnly() {   proxyModel->setFilterActor(".*");   }
    void rowDoubleClicked(const QModelIndex &index);
    void selectionChanged(QModelIndex, QModelIndex);
    void itemClicked(QModelIndex);
    void removeItem();
    void reparseSelection();
    void reparseItem(const QModelIndex &);
    void rightClickMenu(const QPoint &);
    void updateCurrentItem();
private:
    QModelIndex findSceneIndex(const QRegExp &rx, const int column);
    ScenePtr    getSelection(const QModelIndex &);
    void addData(int column, QString data);
    QItemSelectionModel *selectionModel;
    QStandardItemModel  *sceneModel;
    QAbstractItemModel  *sourceModel;
    SceneProxyModel     *proxyModel;
    QStandardItem       *sceneParent;
    QTableView          *table;
    QWidget             *parent;
    QVBoxLayout         *mainLayout;
    int newRow, currentFileSelection;
    bool initComplete;
    QString nameFilter, fileFilter;
    QStringList headers;
    QSharedPointer<DataManager> vault;
    SceneDetailView *detailView;
    ActorProfileView *profileView;
    QModelIndex proxyIndex, modelIndex;
    int currentID;
    QMutex mx;
signals:
    void rowsFinishedLoading();
    void displayChanged(int);
    void sendSceneCount(int);
    void playFile(int sceneID);
    void sceneClicked(int id);
    void sceneClicked(ScenePtr);
    void sceneSelectionChanged(int id);
    void sceneSelectionChanged(ScenePtr);
    void loadSceneDetails(int id);
    void loadSceneDetails(ScenePtr);
    void progressBegin(QString, int);
    void progressUpdate(int);
    void progressEnd(QString);
    void saveFilterSet(FilterSet);

};

#endif // SCENEVIEW_H
