#include "ActorTableView.h"

ActorTableView::ActorTableView(QWidget *parent):
    QWidget(parent),
    parent(parent){
    this->proxyModel = new ActorProxyModel(this);
    this->table = new QTableView;
    proxyModel->setFilterRole(Qt::DisplayRole);
    table->setModel(proxyModel);
    table->setSortingEnabled(true);
    table->sortByColumn(1, Qt::AscendingOrder);
    table->verticalHeader()->hide();
    table->horizontalHeader()->show();
    table->resizeColumnsToContents();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
}

void ActorTableView::setHorizontalHeaders(QStringList){
}

void ActorTableView::resizeView(){
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
}
void ActorTableView::setSourceModel(QAbstractItemModel *model){
    proxyModel->setSourceModel(model);
}

void ActorTableView::addActor(ActorPtr /*a*/, const QModelIndex &/*parent*/){}
void ActorTableView::filterChanged(QString){}
void ActorTableView::filterChangedEthnicity(QString){}
void ActorTableView::filterChangedHair(QString){}
void ActorTableView::filterChangedSceneCount(ActorProxyModel::NumberFilterType, int){}

