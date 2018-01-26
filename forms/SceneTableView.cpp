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
    connect(table, SIGNAL(clicked(QModelIndex)), this, SLOT(sceneClicked(QModelIndex)));
    connect(table, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(rowDoubleClicked(QModelIndex)));
    connect(proxyModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rowCountChanged(QModelIndex,int,int)));
    connect(proxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowCountChanged(QModelIndex,int,int)));
    this->initComplete = true;
}

void SceneTableView::addRows(RowList rows){
    foreach(Row row, rows){
        sceneModel->appendRow(row);
    }
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
#warning Placeholder for updating Scene Display Row
    qDebug("Placeholder Function in SceneTableView for updating scene Display item");
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

void SceneTableView::sceneClicked(QModelIndex modelIndex){
    qDebug("Clicked");
    int id = proxyModel->data(proxyModel->index(modelIndex.row(), SCENE_ID_COLUMN), Qt::DisplayRole).toInt();
    if (id > 0){
        this->currentFileSelection = id;
        if (vault->contains(id)){
            ScenePtr s = vault->getScene(id);
            if (!s.isNull()){
                emit loadSceneDetails(s);
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

int SceneTableView::countRows(){
    return proxyModel->rowCount();
}

void SceneTableView::updateSceneItem(int id){
    if (vault.contains(id)){
        ScenePtr s = vault->getScene(id);
        qDebug("Updating %s's Display Item", qPrintable(s->getFilename()));
        s->updateQStandardItem();
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

void SceneTableView::resizeSceneView(){
    this->table->resizeColumnsToContents();
    this->table->resizeRowsToContents();
}

void SceneTableView::searchByID(const int &id){
    proxyModel->setFilterID(id);
}
void SceneTableView::actorFilterChanged(ActorPtr a){
    if (!a.isNull()){
        actorFilterChanged(a->getName());
    }
}/*
void SceneTableView::actorFilterChanged(QString name){
    filenameFilterChanged("");
    this->nameFilter = name;
    proxyModel->setFilterRegExp(name);
    proxyModel->setFilterActor(name);
    table->resizeColumnsToContents();
}*/

//void SceneTableView::filenameFilterChanged(QString word){
//    this->fileFilter = word;
//    proxyModel->setFilterRegExp(".*"+word+".*");
//    proxyModel->setFilterFilename(word);
//}

void SceneTableView::addData(int column, QString data){
    proxyModel->setData(proxyModel->index(newRow, column), data);
}

void SceneTableView::addNewScene(ScenePtr s){
    if (!s.isNull()){
        if (!vault->contains(s)){
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
    emit progressBegin(QString("Adding %1 Items to the Scene Table").arg(list.size()));
    int index = 0;
    foreach(ScenePtr s, list){
        addScene(s);
        emit progressUpdate(++index);
    }
    emit progressEnd(QString("Added %1 items to the Scene Table").arg(list.size()));
}
