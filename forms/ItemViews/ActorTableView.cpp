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
    actorHeaders << "" << "Name" << "Hair Color" << "Ethnicity" << "Age" << "Height" << "Weight" << "Tattoos?" << "Piercings?" << "Delete";
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
    connect(table,          SIGNAL(customContextMenuRequested(QPoint)),         this, SLOT(displayRightClickMenu(QPoint)));
    /// Hide Columns that are present only for filtering reasons.
    table->hideColumn(ACTOR_WEIGHT_COLUMN);
    table->hideColumn(ACTOR_ETH_COLUMN);
    table->hideColumn(ACTOR_PIERCING_COLUMN);
    table->hideColumn(ACTOR_TATTOO_COLUMN);
}

void ActorTableView::connectViews(SceneTableView *table, SceneDetailView *detail, ActorProfileView *profile){
    this->profileView = profile;
    this->detailView = detail;
    this->sceneTableView = table;
    connect(this,       SIGNAL(actorClicked(QString)),          sceneTableView, SLOT(actorFilterChanged(QString)));
    connect(this,       SIGNAL(actorSelectionChanged(QString)), sceneTableView, SLOT(actorFilterChanged(QString)));
    connect(detailView, SIGNAL(showActor(QString)),             this,           SLOT(selectActor(QString)));
    connect(detailView, SIGNAL(showActor(ActorPtr)),            this,           SLOT(selectActor(ActorPtr)));
    connect(this,       SIGNAL(actorClicked(QString)),          profileView,    SLOT(showProfileView(QString)));
    connect(this,       SIGNAL(actorSelectionChanged(QString)), profileView,    SLOT(updateProfileView(QString)));
    connect(profileView,SIGNAL(requestActor(QString)),          this,           SLOT(selectActor(QString)));
    connect(profileView,SIGNAL(selectActor(QString)),           this,           SLOT(selectActor(QString)));
    connect(profileView,SIGNAL(deleteActor(ActorPtr)),          this,           SLOT(removeActor(ActorPtr)));
    connect(profileView,SIGNAL(deleteActor(QString)),           this,           SLOT(removeActor(QString)));
}

void ActorTableView::addDeleteButtons(){
    qDebug("Adding Delete Buttons");
    for (int r = 0; r < actorModel->rowCount(); ++r){
        QModelIndex index = actorModel->index(r, ACTOR_DELETE_COLUMN);
        QString name = actorModel->data(actorModel->index(r, ACTOR_NAME_COLUMN)).toString();
        QToolButton *t = new QToolButton();
        t->setText(name);
        t->setIcon(QIcon(":/Icons/red_close_icon.png"));
        t->setToolButtonStyle(Qt::ToolButtonIconOnly);
        connect(t, &QToolButton::released, this, [=]{ vault->remove(t->text()); emit deleteActor(t->text()); removeActor(t->text()); });
        table->setIndexWidget(index, t);
    }
}

void ActorTableView::displayRightClickMenu(const QPoint &p){
    QMenu *menu = new QMenu;
    currentIdx = table->indexAt(p);
    if (currentIdx.isValid()){
        QString name = proxyModel->data(proxyModel->index(currentIdx.row(), ACTOR_NAME_COLUMN), Qt::DisplayRole).toString();
        if (!name.isEmpty()){
            current = vault->getActor(name);
            if (!current.isNull()){
                menu->addAction("Remove", this, SLOT(removeCurrent()));
                menu->addAction("Update Bio",       [=] { emit updateFromWeb(current->getName());   });
                menu->addAction("Download Photo",   [=] { emit downloadPhoto(current->getName());   });
                //menu->addAction("Remove Photo",     [=] { current->
                menu->exec(QCursor::pos());
            }
        }
    }
}
void ActorTableView::removeCurrent(){
    if (!current.isNull() && currentIdx.isValid()){
        QModelIndex modelIndex = proxyModel->mapToSource(currentIdx);
        proxyModel->removeRow(currentIdx.row());
        actorModel->removeRow(modelIndex.row());
        vault->remove(current->getName());
        profileView->hide();
    }
}

void ActorTableView::addNewActors(const ActorList &list){
    qDebug("%s adding actors to display", __FILE__);
    emit progressBegin(QString("Adding %1 actors to the view").arg(list.size()), list.size());
    int index = 0;
    bool firstAdd = this->actorModel->hasChildren();
    foreach(ActorPtr a, list){
        if (!a.isNull()){
            int matchingRows = actorModel->findItems(a->getName(),Qt::MatchExactly, ACTOR_NAME_COLUMN).size();
            if (matchingRows == 0){
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

void ActorTableView::addRows(const QVector<QList<QStandardItem *> > rows){
    int index = 0;
    qDebug("%s::%s adding %d rows to the Actor display", __FILE__, __FUNCTION__, rows.size());
    emit progressBegin(QString("Adding %1 rows to the Actor Table").arg(rows.size()), rows.size());
    foreach(QList<QStandardItem *> row, rows){
        this->actorModel->appendRow(row);
        emit progressUpdate(++index);
    }
    emit progressEnd(QString("Finished adding %1 Rows to the display").arg(rows.size()));
    emit rowsFinishedLoading();
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    actorModel->sort(ACTOR_NAME_COLUMN);
}

QString ActorTableView::currentName() const{
    return proxyModel->data(proxyModel->index(table->currentIndex().row(), ACTOR_NAME_COLUMN), Qt::DisplayRole).toString();
}

void ActorTableView::selectionChanged(QModelIndex modelIndex, QModelIndex previousIndex){
    this->prevIdx = previousIndex;
    this->currentIdx = modelIndex;
    QString name = currentName();
    if (!name.isEmpty()){
        if (itemClicked){
            emit actorClicked(name);
        } else {
            emit actorSelectionChanged(name);
        }
    }
    this->currentSelection = name;
    this->itemClicked = false;
}

void ActorTableView::rowClicked(QModelIndex index){
    this->currentIdx = index;
    this->itemClicked = true;
    if (!prevIdx.isValid()){    // If this is the first time an item is selected, open the profile view.
        emit actorClicked(currentName());
   }
}

void ActorTableView::removeActor(QString name){
    qDebug("ActorTableView Removing %s from display...", qPrintable(name));
    QModelIndex index = findActorIndex_Exact(name);
    if (index.isValid()){
        QModelIndex modelIndex = proxyModel->mapToSource(index);
        if (!modelIndex.isValid()){
            qWarning("Error: Mapped Actor Model Index is not valid for the proxy model index given");
        } else {
            actorModel->removeRow(modelIndex.row());
        }
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


void ActorTableView::selectActor(QString name){
    if (!name.isEmpty()){
        QModelIndex i = findActorIndex(name);
        if (i.isValid()){
            table->selectRow(i.row());
        }
    }
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

void ActorTableView::setHorizontalHeaders(QStringList ){
}

void ActorTableView::addActor(ActorPtr a){
    if (!a.isNull()){
        if (actorModel->findItems(a->getName(), Qt::MatchExactly, ACTOR_NAME_COLUMN).isEmpty()){
            actorModel->appendRow(a->buildQStandardItem());
            vault->add(a, true);
        } else {
            vault->update(a);
            a->updateQStandardItem();
        }
    }
}
