#include "SceneDetailView.h"
#include "ui_SceneDetailView.h"

SceneDetailView::SceneDetailView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SceneDetailView)
{
    ui->setupUi(this);
}

SceneDetailView::~SceneDetailView()
{
    delete ui;
}
