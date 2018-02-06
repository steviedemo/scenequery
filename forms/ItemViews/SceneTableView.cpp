#include "SceneTableView.h"
#include "Scene.h"
#include "Actor.h"
#include <QWidget>
#include <QTableView>
#include <QModelIndex>
#include <QStringList>


SceneTableView::SceneTableView(QWidget *parent):parent(parent){
    headers << "Main Actor" << "Title" << "Company" << "Resolution" << "Featured Actors" << "File Size" << "Length" << "Released" << "Rating" << "Location" << "ID";
    this->sceneModel = new QStandardItemModel();
    this->sceneModel->setHorizontalHeaderLabels(headers);
    this->sceneParent = sceneModel->invisibleRootItem();
    this->proxyModel = new SceneProxyModel(this);
    this->proxyModel->setSourceModel(sceneModel);
    this->table = new QTableView;
    proxyModel->setFilterRole(Qt::DisplayRole);
    table->setModel(proxyModel);
    table->setSortingEnabled(true);
    table->verticalHeader()->hide();
    table->horizontalHeader()->show();
    table->setShowGrid(false);
    table->sortByColumn(0, Qt::AscendingOrder);
    table->setCornerButtonEnabled(false);
    table->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    table->setFont(QFont("Futura", 13));
    table->setStatusTip("Scenes");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    table->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    table->setDragDropMode(QAbstractItemView::NoDragDrop);
    this->mainLayout = new QVBoxLayout;
    mainLayout->addWidget(table);
    setLayout(mainLayout);
    //table->setMinimumWidth(800);
    table->resizeColumnsToContents();
    this->selectionModel = table->selectionModel();
    connect(selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChanged(QModelIndex,QModelIndex)));
    connect(table, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
    connect(table, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(rowDoubleClicked(QModelIndex)));
    connect(proxyModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),   this, SLOT(rowCountChanged(QModelIndex,int,int)));
    connect(proxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),  this, SLOT(rowCountChanged(QModelIndex,int,int)));
    connect(table, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(rightClickMenu(QPoint)));
    this->initComplete = true;
}

void SceneTableView::connectViews(SceneDetailView *detail, ActorProfileView *profile){
    this->detailView = detail;
    this->profileView= profile;
    //connect(this,           &SceneTableView::displayChanged,        [=]{    table->resizeColumnsToContents();   });
    connect(this,           SIGNAL(sceneClicked(ScenePtr)),         detailView, SLOT(showDetailView(ScenePtr)));
    connect(this,           SIGNAL(sceneSelectionChanged(ScenePtr)),detailView, SLOT(updateDetailView(ScenePtr)));
    connect(profileView,    SIGNAL(hidden()),                       detailView, SLOT(hideDetailView()));
    connect(profileView,    SIGNAL(hidden()),                       this,       SLOT(setFilter_name()));
    connect(detailView,     SIGNAL(showActor(ActorPtr)),            profileView,SLOT(loadActorProfile(ActorPtr)));
}

void SceneTableView::rightClickMenu(const QPoint &p){
    proxyIndex = table->indexAt(p);
    if (proxyIndex.isValid()){
        modelIndex = proxyModel->mapToSource(proxyIndex);
        currentID = proxyModel->data(proxyModel->index(proxyIndex.row(), SCENE_ID_COLUMN), Qt::DisplayRole).toInt();
        QMenu *menu = new QMenu;
        menu->addAction(QIcon(":/Icons/red_close_icon.png"), "Remove", this, SLOT(removeItem()));
        menu->addAction(QIcon(":/Icons/shiny_blue_reload_icon.png"), "Reparse", this, SLOT(reparseItem()));
        menu->exec();
    }
}

void SceneTableView::removeItem(){
    if (proxyIndex.isValid() && modelIndex.isValid()){
        proxyModel->removeRow(proxyIndex.row());
        sceneModel->removeRow(modelIndex.row());
        vault->remove(currentID);
        qDebug("Item Removed");
    } else {
        qWarning("Can't Remove Scene Item unless the Model Indexes Given are valid! The Rows Given are [Proxy]: %d, [Model]: %d", proxyIndex.row(), modelIndex.row());
    }
}

void SceneTableView::reparseItem(){
    if (proxyIndex.isValid()){
        ScenePtr s = getSelection(proxyIndex);
        s->reparse();
        s->updateQStandardItem();
    } else {
        qWarning("Can't Reparse Scene Item With invalid index!");
    }
}

ScenePtr SceneTableView::getSelection(const QModelIndex &x){
    ScenePtr s;
    if (x.isValid()){
        bool ok = false;
        int id = proxyModel->data(proxyModel->index(x.row(), SCENE_ID_COLUMN)).toInt(&ok);
        if (ok && id > -1){
            s = vault->getScene(id);
        }
    }
    return s;
}


void SceneTableView::addRows(RowList rows){
    int index = 0;
    qDebug("%s::%s adding %d rows to the Scene Table", __FILE__, __FUNCTION__, rows.size());
    emit progressBegin(QString("Adding %1 Rows to the actor Table").arg(rows.size()), rows.size());
    foreach(Row row, rows){
        this->sceneModel->appendRow(row);
        emit progressUpdate(++index);
    }
    emit progressEnd(QString("Finished Adding %1 Rows to the Scene Table").arg(rows.size()));
    emit rowsFinishedLoading();
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    sceneModel->sort(SCENE_NAME_COLUMN);
}


void SceneTableView::rowCountChanged(QModelIndex, int, int){
    emit displayChanged(proxyModel->rowCount());
}

QModelIndex SceneTableView::findSceneIndex(const QRegExp &rx, const int column){
    QAbstractItemModel *model = this->table->model();
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(model);
    proxy.setFilterKeyColumn(column);
    proxy.setFilterRegExp(rx);
    QModelIndex index = proxy.mapToSource(proxy.index(0,0));
    if (!index.isValid()){
        index = QModelIndex();
        qWarning("Unable to find Scene with column %d matching '%s'", column, qPrintable(rx.pattern()));
    }
    return index;
}

void SceneTableView::updateSceneDisplay(int id){
    if(vault->contains(id)){
        vault->getScene(id)->updateQStandardItem();
    }
}

void SceneTableView::updateSceneItem(int id){
    if (vault->contains(id)){
        ScenePtr s = vault->getScene(id);
        qDebug("Updating %s's Display Item", qPrintable(s->getFilename()));
        s->updateQStandardItem();
    }
}


void SceneTableView::selectionChanged(QModelIndex modelIndex, QModelIndex /*oldIndex*/){
    qDebug("Selection Changed");
    int id = proxyModel->data(proxyModel->index(modelIndex.row(), SCENE_ID_COLUMN), Qt::DisplayRole).toInt();
    if (id > 0){
        if (currentFileSelection != id){
            this->currentFileSelection = id;
            ScenePtr s = vault->getScene(id);
            if (!s.isNull()){
                emit sceneSelectionChanged(s);
            }
        }
    }
}

void SceneTableView::itemClicked(QModelIndex modelIndex){
    qDebug("Clicked");
    int id = proxyModel->data(proxyModel->index(modelIndex.row(), SCENE_ID_COLUMN), Qt::DisplayRole).toInt();
    if (id > 0){
        this->currentFileSelection = id;
        if (vault->contains(id)){
            ScenePtr s = vault->getScene(id);
            if (!s.isNull()){
                emit sceneClicked(s);
            }
        }
    }
}

void SceneTableView::rowDoubleClicked(const QModelIndex &modelIndex){
    int id = proxyModel->data(proxyModel->index(modelIndex.row(), SCENE_ID_COLUMN), Qt::DisplayRole).toInt();
    if (id > 0){
        this->currentFileSelection = id;
        emit playFile(id);
    }
}

void SceneTableView::receiveSceneCountRequest(){
    emit sendSceneCount(proxyModel->rowCount());
}

void SceneTableView::setSourceModel(QAbstractItemModel *model){
    this->sourceModel = model;
    proxyModel->setSourceModel(model);
}

QVector<int> SceneTableView::getIDs(){
    QVector<int> ids = {};
    qDebug("SceneTableView receievd Request for a list of the show scenes' IDs");
    for(int i = 0; i < proxyModel->rowCount(); ++i){
        QModelIndex index = proxyModel->index(i, SCENE_ID_COLUMN);
        int id = proxyModel->data(index, Qt::DisplayRole).toInt();
        ids.push_back(id);
        qDebug("Adding Scene With ID '%d'", id);
    }
    return ids;
}

void SceneTableView::searchByID(const int &id){
    proxyModel->setFilterID(id);
}
void SceneTableView::setFilter_name(const ActorPtr a){
    if (!a.isNull()){
        proxyModel->setFilterActor(a->getName());
    }
}

void SceneTableView::loadFilters(FilterSet set){
    if (set.getFilterType() == ACTOR_FILTER){
        qWarning("Scene Table View received filter set of type 'Actor Filter'. Not Loading.");
        return;
    } else {
        proxyModel->loadFilters(set);
    }
}
void SceneTableView::saveFilters(){
    FilterSet filters = FilterSet(this->proxyModel);
    emit saveFilterSet(filters);
}

void SceneTableView::addData(int column, QString data){
    proxyModel->setData(proxyModel->index(newRow, column), data);
}

void SceneTableView::addNewScene(ScenePtr s){
    if (!s.isNull()){
        if (!vault->contains(s->getID())){
            sceneModel->appendRow(s->buildQStandardItem());
            vault->add(s);
        }
    }
}

void SceneTableView::purgeSceneItems(QVector<int> ids){
    qDebug("Looking for %d Scenes in the GUI Display list...", ids.size());
    QVector<QModelIndex> removals = {};
    for (int r = 0; r < sceneModel->rowCount(); ++r){
        QModelIndex curr = sceneModel->index(r, SCENE_ID_COLUMN);
        if (curr.isValid()){
            int id = curr.data().toInt();
            if (ids.contains(id)){
                removals << curr;
            }
        }
    }
    qDebug("Found %d/%d scenes in the Display List", removals.size(), ids.size());
    foreach(QModelIndex idx, removals){
        sceneModel->removeRow(idx.row());
    }
    qDebug("Scenes removed");
}

void SceneTableView::addNewScenes(SceneList list){
    emit progressBegin(QString("Adding %1 Items to the Scene Table").arg(list.size()), list.size());
    int index = 0;
    foreach(ScenePtr s, list){
        addNewScene(s);
        emit progressUpdate(++index);
    }
    emit progressEnd(QString("Added %1 items to the Scene Table").arg(list.size()));
}
