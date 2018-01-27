#include "FilterWidget.h"
#include "ui_FilterWidget.h"

FilterWidget::FilterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterWidget)
{
    ui->setupUi(this);
    initCb(ui->cb_addedDate);
    initCb(ui->cb_age);
    initCb(ui->cb_duration);
    initCb(ui->cb_height);
    initCb(ui->cb_ratingOp);
    initCb(ui->cb_releaseDate);
    initCb(ui->cb_weight);

    connect(ui->pb_actorFilters, &QPushButton::clicked, [=]{ ui->stackedWidget->setCurrentIndex(0);});
    connect(ui->pb_scenes,      &QPushButton::clicked,  [=]{ ui->stackedWidget->setCurrentIndex(1);});
}

void FilterWidget::initCb(QComboBox *cb){
    QStringList operators;
    operators << "" << "Equals" << "Doesn't Equal" << "Is Less/More Recent Than" << "Less than or Equal to" << "Greater Than" << "Greater than or Equal to";
    for(int i = 0; i < operators.size(); ++i){
        cb->insertItem(i, operators.at(i));
    }
}

void FilterWidget::setEthnicityValues(QStringList &l){
    ui->cb_skin->clear();
    for(int i = 0; i < l.size(); ++i){
        ui->cb_skin->insertItem(i, l.at(i));
    }
}

void FilterWidget::setHairValues(QStringList &l){
    ui->cb_hair->clear();
    for(int i = 0; i < l.size(); ++i){
        ui->cb_hair->insertItem(i, l.at(i));
    }
}

FilterWidget::~FilterWidget(){
    delete ui;
}
