#include "ActorTableView.h"
#include "definitions.h"
#include "Actor.h"
ActorTableView::ActorTableView(QWidget *parent):
    QWidget(parent),
    parent(parent){
    this->itemClicked = false;
    this->proxyModel = new ActorProxyModel(this);
    this->table = new QTableView;
    proxyModel->setFilterRole(Qt::DisplayRole);
    table->setModel(proxyModel);
    table->setSortingEnabled(true);
    table->sortByColumn(1, Qt::AscendingOrder);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(24);
    table->horizontalHeader()->setDefaultSectionSize(70);
    table->horizontalHeader()->show();
    table->setShowGrid(false);
    table->sortByColumn(ACTOR_NAME_COLUMN);
    table->setCornerButtonEnabled(false);
    table->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    table->setFont(QFont("Futura", 13));
    table->setStatusTip("Actors");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->resizeColumnsToContents();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    table->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    table->setDragDropMode(QAbstractItemView::NoDragDrop);
    table->setFrameShape(QFrame::NoFrame);
    table->setFrameShadow(QFrame::Plain);
    table->setIconSize(QSize(20, 20));
    table->setGridStyle(Qt::DotLine);

    this->mainLayout = new QVBoxLayout;
    mainLayout->addWidget(table);
    setLayout(mainLayout);
    this->selectionModel = table->selectionModel();
    connect(selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChanged(QModelIndex,QModelIndex)));
    connect(table, SIGNAL(clicked(QModelIndex)), this, SLOT(rowClicked(QModelIndex)));
    connect(proxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowCountChanged(QModelIndex,int,int)));
    connect(proxyModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),  this, SLOT(rowCountChanged(QModelIndex,int,int)));
}
void ActorTableView::rowCountChanged(QModelIndex, int, int){
    emit displayChanged(proxyModel->rowCount());
}
void ActorTableView::resizeToContents(){
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
}

QString ActorTableView::selectedName() const{
    return proxyModel->data(proxyModel->index(table->currentIndex().row(), ACTOR_NAME_COLUMN), Qt::DisplayRole).toString();
}

QModelIndex ActorTableView::currentIndex(){
    return table->currentIndex();
}

void ActorTableView::selectionChanged(QModelIndex modelIndex, QModelIndex){
    this->prevIdx = currentIdx;
    this->currentIdx = modelIndex;
    QString name = selectedName();
    if (!name.isEmpty()){
        if (itemClicked){
            emit actorClicked(name);
            itemClicked = false;
        } else {
            emit actorSelectionChanged(name);
        }
    }
    this->currentSelection = name;
}

void ActorTableView::rowClicked(QModelIndex /*idx*/){
    itemClicked = true;
    if (!prevIdx.isValid()){
        emit actorClicked(selectedName());
    }
}
void ActorTableView::removeActor(ActorPtr a){
    removeActor(a->getName());
}

void ActorTableView::removeActor(QString name){
    qDebug("ActorTableView Removing %s from display...", qPrintable(name));
    QModelIndex index = findActorIndex_Exact(name);
    if (index.isValid()){
        proxyModel->removeRow(index.row());
    }
}

QModelIndex ActorTableView::findActorIndex(const QString &name) const{
    QString regex = QString("%1").arg(name);
    QRegExp rx(regex, Qt::CaseInsensitive,QRegExp::RegExp);
    return findActorIndex_base(rx, ACTOR_NAME_COLUMN);
}

QModelIndex ActorTableView::findActorIndex_Exact(const QString &name) const{
    QString regex = QString("^%1$").arg(name);
    QRegExp rx(regex, Qt::CaseInsensitive,QRegExp::RegExp);
    return findActorIndex_base(rx, ACTOR_NAME_COLUMN);
}


QModelIndex ActorTableView::findActorIndex_base(const QRegExp &rx, const int column) const{
    QAbstractItemModel *model = table->model();
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(model);
    proxy.setFilterKeyColumn(column);
    proxy.setFilterRegExp(rx);
    QModelIndex matchingIndex = proxy.mapToSource(proxy.index(0,0));
    if (!matchingIndex.isValid()){
        QMessageBox box(QMessageBox::Critical, tr("Error Finding Actor"), QString("Error Locating %1").arg(rx.pattern()), QMessageBox::Abort);
        box.exec();
        matchingIndex = QModelIndex();
    } else {
        int row = matchingIndex.row();
        qDebug("'%s' was found in row %d", qPrintable(rx.pattern()), row);
    }
    return matchingIndex;
}


void ActorTableView::selectActor(const QString &name){
    if (!name.isEmpty()){
        QModelIndex i = findActorIndex(name);
        if (i.isValid()){
            table->selectRow(i.row());
        }
    }
}

int ActorTableView::countRows(){
    return proxyModel->rowCount();
}
QStringList ActorTableView::namesDisplayed(){
    QStringList names;
    for(int i = 0; i < proxyModel->rowCount(); ++i){
        QModelIndex index = proxyModel->index(i, ACTOR_NAME_COLUMN);
        QString curr = proxyModel->data(index, Qt::DisplayRole).toString();
        if (!curr.isEmpty()){
            names << curr;
        }
    }
    return names;
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
void ActorTableView::filterChangedName(const QString &name){
    this->nameFilter = name;
    proxyModel->setFilterName(name);
}

void ActorTableView::filterChangedEthnicity(QString s){
    this->ethnicityFilter = s;
    proxyModel->setFilterEthnicity(s);
}
void ActorTableView::filterChangedHair(QString s){
    this->hairFilter = s;
    proxyModel->setFilterHairColor(s);
}
void ActorTableView::filterChangedSceneCount(ActorProxyModel::NumberFilterType, int){}

