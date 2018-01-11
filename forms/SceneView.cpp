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
    table->verticalHeader()->hide();
    table->horizontalHeader()->show();
    table->setShowGrid(false);
    table->setSortingEnabled(true);
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
    connect(table, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(rowDoubleClicked(QModelIndex)));
}

void SceneView::rowDoubleClicked(const QModelIndex &sourceParent){
    QString filepath = proxyModel->data(proxyModel->index(sourceParent.row(), SCENE_PATH_COLUMN, sourceParent)).toString();
    emit playFile(filepath);
}

QVBoxLayout *SceneView::getLayout(){
    return mainLayout;
}
void SceneView::receiveSceneCountRequest(){
    emit sendSceneCount(proxyModel->rowCount());
}

void SceneView::setSourceModel(QAbstractItemModel *model){
    proxyModel->setSourceModel(model);
}

void SceneView::resizeSceneView(){
    this->table->resizeColumnsToContents();
    this->table->resizeRowsToContents();
}

void SceneView::clearFilter(){
    actorFilterChanged("");
}
void SceneView::actorFilterChanged(ActorPtr a){
    actorFilterChanged(a->getName());
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

void SceneView::actorFilterChanged(QString name){
    proxyModel->setFilterRegExp(name);
    proxyModel->setFilterActor(name);
    table->resizeColumnsToContents();
}
void SceneView::addData(int column, QString data){
    proxyModel->setData(proxyModel->index(newRow, column), data);
}

void SceneView::addScene(ScenePtr s, const QModelIndex &parent){
    if (!s.isNull()){
        Scene::RowData data = s->getRowData();
        newRow = proxyModel->rowCount();
        proxyModel->insertRow(newRow, parent);
        addData(SCENE_NAME_COLUMN, data.mainActor);
        addData(SCENE_TITLE_COLUMN, data.title);
        addData(SCENE_COMPANY_COLUMN, data.company);
        addData(SCENE_QUALITY_COLUMN, data.quality);
        addData(SCENE_FEATURED_COLUMN, data.featured);
        addData(SCENE_DATE_COLUMN, data.date);
        addData(SCENE_LENGTH_COLUMN, data.length);
        addData(SCENE_RATING_COLUMN, data.rating);
    }
}
