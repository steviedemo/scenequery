#include "SceneDetailView.h"
#include "ui_SceneDetailView.h"
#include <QList>
#include <QtGui>
#include <QLabel>
#include <QListIterator>
SceneDetailView::SceneDetailView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SceneDetailView)
{
    ui->setupUi(this);
    for (int i = 0; i < 4; ++i){
        QLabel *label = new QLabel("");
        label->setMouseTracking(true);
        label->setOpenExternalLinks(false);
        label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        connect(castList.at(i), SIGNAL(linkHovered(QString)), this, SLOT(actorLinkHovered(QString)));
        connect(castList.at(i), SIGNAL(linkActivated(QString)), this, SLOT(actorLinkClicked(QString)));
        castList << label;
    }
}

SceneDetailView::~SceneDetailView(){
    delete ui;
    QListIterator<QLabel*> labelIt = castList.end();
    while(labelIt.hasPrevious()){
        QLabel *curr = labelIt.previous();
        delete curr;
    }
}
void SceneDetailView::clearLinks(){
    foreach(QLabel*link, castList){
        link->setText("");
    }
}

void SceneDetailView::generateLinks(QStringList cast){
    for (int i = 0; i < cast.size(); ++i){
        QString s = cast.at(i);
        if (!s.isEmpty()){
            QString link = QString("<a href=\"%1\">%1</a>").arg(cast.at(i));
            castList.at(i)->setText(link);
        }
    }
}
void SceneDetailView::actorLinkHovered(QString name){
    foreach(QLabel *label, castList){
        if (label->text().contains(name)){
        }
    }
}
void SceneDetailView::actorLinkClicked(QString name){
    if (!name.isEmpty()){
        emit showActor(name);
    }
}
