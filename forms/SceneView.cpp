#include "SceneView.h"
#include "Scene.h"
#include "Actor.h"
#include <QTableView>
#include <QModelIndex>
SceneView::SceneView(QWidget *parent):
    parent(parent),
    newRow(0){
    this->proxyModel = new SceneProxyModel(this);
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

void SceneView::rowCountChanged(QModelIndex, int, int){
    emit displayChanged(proxyModel->rowCount());

}

QModelIndex SceneView::findSceneIndex(const QRegExp &rx, const int column){
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

void SceneView::updateSceneDisplay(int id){
#warning Placeholder for updating Scene Display Row
    qDebug("Placeholder Function in SceneView for updating scene Display item");
}

void SceneView::selectionChanged(QModelIndex modelIndex, QModelIndex /*oldIndex*/){
    qDebug("Selection Changed");
    int id = proxyModel->data(proxyModel->index(modelIndex.row(), SCENE_ID_COLUMN), Qt::DisplayRole).toInt();
    if (id > 0){
        if (currentFileSelection != id){
            this->currentFileSelection = id;
            emit sceneSelectionChanged(id);
        }
    }
}

void SceneView::sceneClicked(QModelIndex modelIndex){
    qDebug("Clicked");
    int id = proxyModel->data(proxyModel->index(modelIndex.row(), SCENE_ID_COLUMN), Qt::DisplayRole).toInt();
    if (id > 0){
        this->currentFileSelection = id;
        emit sceneItemClicked(id);
    }
}

void SceneView::rowDoubleClicked(const QModelIndex &modelIndex){
    int id = proxyModel->data(proxyModel->index(modelIndex.row(), SCENE_ID_COLUMN), Qt::DisplayRole).toInt();
    if (id > 0){
        this->currentFileSelection = id;
        emit playFile(id);
    }
}

int SceneView::countRows(){
    return proxyModel->rowCount();
}

void SceneView::updateSceneItem(int id){

}

void SceneView::receiveSceneCountRequest(){
    emit sendSceneCount(proxyModel->rowCount());
}

void SceneView::setSourceModel(QAbstractItemModel *model){
    proxyModel->setSourceModel(model);
}

QVector<int> SceneView::getIDs(){
    QVector<int> ids = {};
    qDebug("SceneView receievd Request for a list of the show scenes' IDs");
    for(int i = 0; i < proxyModel->rowCount(); ++i){
        QModelIndex index = proxyModel->index(i, SCENE_ID_COLUMN);
        int id = proxyModel->data(index, Qt::DisplayRole).toInt();
        ids.push_back(id);
        qDebug("Adding Scene With ID '%d'", id);
    }
    return ids;
}

void SceneView::receiveRequestForShownSceneIDs(){
    emit sendSceneIDs(getIDs());
}

void SceneView::resizeSceneView(){
    this->table->resizeColumnsToContents();
    this->table->resizeRowsToContents();
}

void SceneView::clearFilter(){
    actorFilterChanged("");
}
void SceneView::clearActorFilterOnly(){
    this->nameFilter = "";
    proxyModel->setFilterActor("");
}

void SceneView::searchByFilename(const QString &searchTerm){
    this->filenameFilterChanged(searchTerm);
}
void SceneView::clearSearchFilter(){
    this->filenameFilterChanged("");
}

void SceneView::searchByID(const int &id){
    proxyModel->setFilterID(id);
}
void SceneView::actorFilterChanged(ActorPtr a){
    actorFilterChanged(a->getName());
}
void SceneView::actorFilterChanged(QString name){
    filenameFilterChanged("");
    this->nameFilter = name;
    proxyModel->setFilterRegExp(name);
    proxyModel->setFilterActor(name);
    table->resizeColumnsToContents();
}

void SceneView::companyFilterChanged(QString company){
    proxyModel->setFilterRegExp(company);
    proxyModel->setFilterCompany(company);
}

void SceneView::tagFilterChanged(QString tag){
    proxyModel->setFilterRegExp(tag);
    proxyModel->setFilterTag(tag);
}

void SceneView::qualityFilterChanged(int resolution){
    proxyModel->setFilterRegExp(QString::number(resolution));
    proxyModel->setFilterQuality(resolution);
}

void SceneView::filenameFilterChanged(QString word){
    this->fileFilter = word;
    proxyModel->setFilterRegExp(".*"+word+".*");
    proxyModel->setFilterFilename(word);
}

void SceneView::addData(int column, QString data){
    proxyModel->setData(proxyModel->index(newRow, column), data);
}

void SceneView::addScene(ScenePtr s, const QModelIndex &/*parent*/){
    if (!s.isNull()){
//        Scene::RowData data = s->getRowData();
//        newRow = proxyModel->rowCount();
//        proxyModel->insertRow(newRow, parent);
//        addData(SCENE_NAME_COLUMN, data.mainActor);
//        addData(SCENE_TITLE_COLUMN, data.title);
//        addData(SCENE_COMPANY_COLUMN, data.company);
//        addData(SCENE_QUALITY_COLUMN, data.quality);
//        addData(SCENE_FEATURED_COLUMN, data.featured);
//        addData(SCENE_DATE_COLUMN, data.date);
//        addData(SCENE_LENGTH_COLUMN, data.length);
//        addData(SCENE_RATING_COLUMN, data.rating);
    }
}
