#ifndef SCENEVIEW_H
#define SCENEVIEW_H
#include <QtWidgets>
#include <QWidget>
#include <QStandardItemModel>
#include "SceneProxyModel.h"
#include "DataManager.h"
#include <QSharedPointer>
#include "SceneList.h"
#include "definitions.h"
class SceneTableView : public QWidget
{
    Q_OBJECT
public:
    explicit SceneTableView(QWidget *parent = 0);
    void setSourceModel(QAbstractItemModel *model);
    QVector<int> getIDs();
    void updateSceneDisplay(int);
    int countRows();
    void setDataContainers(QSharedPointer<DataManager> vault){  this->vault = vault;    }
public slots:
    void addRows(RowList sceneRows);
    void addNewScene(ScenePtr);
    void addNewScenes(SceneList);
    void receiveSceneCountRequest();
    void resizeSceneView();
    void updateSceneItem(int id);
    void purgeSceneItems(QVector<int>);

    void actorFilterChanged(const ActorPtr a);
    void actorFilterChanged(const QString s)        {   proxyModel->setFilterActor(QString("^.*%1.*$").arg(s)); }
    void companyFilterChanged(QString s="")         {   proxyModel->setFilterCompany(QString(".*%1.*").arg(s)); }
    void tagFilterChanged(QString s="")             {   proxyModel->setFilterTag(QString(".*%1.*").arg(s)); }
    void filenameFilterChanged(const QString s="")       {   proxyModel->setFilterFilename(QString(".*%1.*").arg(s));    }
    void qualityFilterChanged   (const int i=-1,  const LogicalOperator op=NOT_SET) {   proxyModel->setFilterQuality(i, op);    }
    void durationFilterChanged  (QTime t=QTime(), const LogicalOperator op=NOT_SET){  proxyModel->setFilterDuration(t, op);   }
    void searchByID(const int &);
    void clearSearchFilter()                        {   proxyModel->setFilterFilename(".*"); }
private slots:
    void rowCountChanged(QModelIndex, int, int);
    void clearFilter(void)      {   proxyModel->clearFilters();         }
    void clearActorFilterOnly() {   proxyModel->setFilterActor(".*");   }
    void rowDoubleClicked(const QModelIndex &index);
    void selectionChanged(QModelIndex, QModelIndex);
    void sceneClicked(QModelIndex);
private:
    QModelIndex findSceneIndex(const QRegExp &rx, const int column);
    void addData(int column, QString data);
    QItemSelectionModel *selectionModel;
    QStandardItemModel  *sceneModel;
    QAbstractItemModel *sourceModel;
    SceneProxyModel *proxyModel;
    QStandardItem *sceneParent;
    QTableView *table;
    QWidget *parent;
    QVBoxLayout *mainLayout;
    int newRow, currentFileSelection;
    bool initComplete;
    QString nameFilter, fileFilter;
    QStringList headers;
    QSharedPointer<DataManager> vault;
signals:
    void displayChanged(int);
    void sendSceneCount(int);
    void playFile(int sceneID);
    void sceneSelectionChanged(ScenePtr);
    void loadSceneDetails(ScenePtr);
    void progressBegin(QString, int);
    void progressUpdate(int);
    void progressEnd(QString);
};

#endif // SCENEVIEW_H
