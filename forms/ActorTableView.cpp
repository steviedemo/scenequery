#include "ActorTableView.h"
#include "definitions.h"
#include "Actor.h"
ActorTableView::ActorTableView(QWidget *parent):
    QWidget(parent),
    parent(parent){

    this->itemClicked = false;
    this->proxyModel = new ActorProxyModel(this);
    this->table = new QTableView;
    QStringList actorHeaders;
    actorHeaders << "" << "Name" << "Hair Color" << "Ethnicity" << "Age" << "Height" << "Weight" << "Tattoos?" << "Piercings?";
    this->actorModel = new QStandardItemModel();
    this->actorModel->setSortRole(Qt::DecorationRole);
    this->actorParent = actorModel->invisibleRootItem();
    this->actorModel->setHorizontalHeaderLabels(actorHeaders);
    proxyModel->setSourceModel(actorModel);
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
    /// Set Up Selection Model
    this->selectionModel = table->selectionModel();
    connect(selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChanged(QModelIndex,QModelIndex)));
    connect(table,          SIGNAL(clicked(QModelIndex)),                       this, SLOT(rowClicked(QModelIndex)));
    connect(proxyModel,     SIGNAL(rowsInserted(QModelIndex,int,int)),          this, SLOT(rowCountChanged(QModelIndex,int,int)));
    connect(proxyModel,     SIGNAL(rowsRemoved(QModelIndex,int,int)),           this, SLOT(rowCountChanged(QModelIndex,int,int)));
    /// Hide Columns that are present only for filtering reasons.
    table->hideColumn(ACTOR_WEIGHT_COLUMN);
    table->hideColumn(ACTOR_ETH_COLUMN);
    table->hideColumn(ACTOR_PIERCING_COLUMN);
    table->hideColumn(ACTOR_TATTOO_COLUMN);
}

void ActorTableView::addRows(RowList rows){
    foreach(Row row, rows){
        actorModel->appendRow(row);
    }
    actorModel->sort(ACTOR_NAME_COLUMN);
}

void ActorTableView::addNewActors(const ActorList &list){
    qDebug("%s adding actors to display", __FILE__);
    emit progressBegin(QString("Adding %1 actors to the view").arg(list.size()), list.size());
    int index = 0;
    bool firstAdd = this->actorModel->hasChildren();
    foreach(ActorPtr a, list){
        if (!a.isNull()){
            int matchingRows = actorModel->findItems(a->getName(),Qt::MatchExactly, ACTOR_NAME_COLUMN).size();
            if (firstAdd || (matchingRows == 0)){
                actorModel->appendRow(a->buildQStandardItem());
                vault->add(a);
            } else if (!firstAdd){
                vault->update(a);
            }
        } else {
            qWarning("%s::%s element %d of actor list is null!", __FILE__, __FUNCTION__, index);
        }
        emit progressUpdate(++index);
    }
    emit progressEnd("Finished Adding actors to display");
    table->sortByColumn(ACTOR_NAME_COLUMN);
}

void ActorTableView::addNewActor(const ActorPtr a){
    if (!a.isNull()){
        qDebug("Adding %s to the display...", qPrintable(a->getName()));
        if (!vault->contains(a->getName())){
            actorModel->appendRow(a->buildQStandardItem());
            vault->add(a);
            qDebug("%s Added to the vault and to the display", qPrintable(a->getName()));
        }
    } else {
        qWarning("Error: %s::%s Attempting to add Null ActorPtr to display", __FILE__, __FUNCTION__);
    }
}

void ActorTableView::addNewItems(const QVector<QList<QStandardItem *> > rows){
    int index = 0;
    qDebug("%s::%s adding %d rows to display", __FILE__, __FUNCTION__, rows.size());
    emit progressBegin(QString("Adding %1 rows to the Actor Table").arg(rows.size()), rows.size());
    foreach(QList<QStandardItem *> row, rows){
        this->actorModel->appendRow(row);
        emit progressUpdate(++index);
    }
    emit progressEnd(QString("Finished adding %1 Rows to the display").arg(rows.size()));
}

void ActorTableView::rowCountChanged(QModelIndex, int, int){
    emit displayChanged(proxyModel->rowCount());
}
void ActorTableView::resizeToContents(){
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    table->sortByColumn(ACTOR_NAME_COLUMN);
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

void ActorTableView::addActor(ActorPtr a){
    if (!a.isNull() && actorModel->findItems(a->getName(), Qt::MatchExactly, ACTOR_NAME_COLUMN).isEmpty()){
        actorModel->appendRow(a->buildQStandardItem());
        vault->add(a, true);
    }
}
void ActorTableView::filterChanged(QString){}
void ActorTableView::filterChangedName(const QString name){
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

