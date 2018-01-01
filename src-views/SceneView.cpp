#include "SceneView.h"
#include "Scene.h"
#include <QTableView>
#define NAME_COLUMN 0
#define TITLE_COLUMN    1
#define COMPANY_COLUMN  2
#define QUALITY_COLUMN  3
#define FEATURED_COLUMN 4
#define DATE_COLUMN     5
#define LENGTH_COLUMN   6
#define RATING_COLUMN   7
SceneView::SceneView(QWidget *parent):
    parent(parent),
    newRow(0){
    headers << "Main Actor" << "Title" << "Company" << "Quality" << "Featured Actors" << "Release Date" << "Length" << "Rating";
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
    proxyView->resizeColumnsToContents();
}
QVBoxLayout *SceneView::getLayout(){
    return mainLayout;
}

void SceneView::setSourceModel(QAbstractItemModel *model){
    proxyModel->setSourceModel(model);
}

void SceneView::actorFilterChanged(QString name){
    proxyModel->setFilterFixedString(name);
    proxyModel->setFilterActor(name);
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
