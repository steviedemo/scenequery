#include "FilterWidget.h"
#include "ui_FilterWidget.h"

FilterWidget::FilterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterWidget)
{
    ui->setupUi(this);
    connect(ui->pb_actorFilters, &QPushButton::clicked, [=]{ ui->stackedWidget->setCurrentIndex(0);});
    connect(ui->pb_scenes,      &QPushButton::clicked,  [=]{ ui->stackedWidget->setCurrentIndex(1);});
}

FilterWidget::~FilterWidget()
{
    delete ui;
}

void FilterWidget::on_pb_actorFilters_clicked(){
    ui->stackedWidget->setCurrentIndex(0);
}
