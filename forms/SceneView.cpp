#include "SceneView.h"
#include "Scene.h"
#include "Actor.h"
#include <QTableView>
#include <QModelIndex>
SceneView::SceneView(QWidget *parent):
    parent(parent),
    newRow(0){
    this->proxyModel = new SceneProxyModel(this);
    this->proxyView = new QTableView;
    proxyModel->setFilterRole(Qt::DisplayRole);
    proxyView->setModel(proxyModel);
    proxyView->setSortingEnabled(true);
    proxyView->sortByColumn(0, Qt::AscendingOrder);
    proxyView->verticalHeader()->hide();
    proxyView->horizontalHeader()->show();
    this->mainLayout = new QVBoxLayout;
    mainLayout->addWidget(proxyView);
    setLayout(mainLayout);
    //proxyView->setMinimumWidth(800);
    proxyView->resizeColumnsToContents();
    proxyView->setSelectionBehavior(QAbstractItemView::SelectRows);
    proxyView->setSelectionMode(QAbstractItemView::SingleSelection);
    proxyView->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    proxyView->
    connect(proxyView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(rowDoubleClicked(QModelIndex)));
}

void SceneView::rowDoubleClicked(const QModelIndex &sourceParent){
    QString filepath = proxyModel->data(proxyModel->index(sourceParent.row(), PATH_COLUMN, sourceParent)).toString();
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
    this->proxyView->resizeColumnsToContents();
    this->proxyView->resizeRowsToContents();
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
    proxyView->resizeColumnsToContents();
}
void SceneView::addData(int column, QString data){
    proxyModel->setData(proxyModel->index(newRow, column), data);
}

void SceneView::addScene(ScenePtr s, const QModelIndex &parent){
    if (!s.isNull()){
        Scene::RowData data = s->getRowData();
        newRow = proxyModel->rowCount();
        proxyModel->insertRow(newRow, parent);
        addData(NAME_COLUMN, data.mainActor);
        addData(TITLE_COLUMN, data.title);
        addData(COMPANY_COLUMN, data.company);
        addData(QUALITY_COLUMN, data.quality);
        addData(FEATURED_COLUMN, data.featured);
        addData(DATE_COLUMN, data.date);
        addData(LENGTH_COLUMN, data.length);
        addData(RATING_COLUMN, data.rating);
    }
}
