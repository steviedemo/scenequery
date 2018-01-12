#include "SceneDetailView.h"
#include "ui_SceneDetailView.h"
#include <QList>
#include <QtGui>
#include <QLabel>
#include <QListIterator>
#include "Scene.h"
#include "genericfunctions.h"
#include "Rating.h"
SceneDetailView::SceneDetailView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SceneDetailView)
{
    ui->setupUi(this);
    this->castList.push_back(ui->actor1label);
    this->castList.push_back(ui->actor2label);
    this->castList.push_back(ui->actor3label);
    this->castList.push_back(ui->actor4label);
    this->ageList << ui->age1label;
    this->ageList << ui->age2label;
    this->ageList << ui->age3label;
    this->ageList << ui->age4label;
    this->dataFields << ui->titleLineEdit;
    this->dataFields << ui->companyLineEdit;
    this->dataFields << ui->durationLineEdit;
    this->dataFields << ui->releasedLineEdit;
    this->dataFields << ui->resolutionLineEdit;
    this->dataFields << ui->sizeLineEdit;
    this->dataFields << ui->seriesLineEdit;

    for(int i = 0; i < 4; ++i){
        QLabel *label = castList.at(i);
        label->setMouseTracking(true);
        label->setOpenExternalLinks(false);
        label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        connect(castList.at(i), SIGNAL(linkHovered(QString)), this, SLOT(actorLinkHovered(QString)));
        connect(castList.at(i), SIGNAL(linkActivated(QString)), this, SLOT(actorLinkClicked(QString)));
    }
    QStringList ratingValues = getRatingList();
    foreach(QString rating, ratingValues){
        ui->ratingComboBox->addItem(rating, rating);
    }
}

SceneDetailView::~SceneDetailView(){
    delete ui;
}

void SceneDetailView::loadScene(ScenePtr s){
    this->current = s;
    ui->companyLineEdit->setText(s->getCompany());
    QTime duration = s->getLength();
    ui->durationLineEdit->setText(duration.toString("h:mm:ss"));
    QDate released = s->getReleased();
    ui->releasedLineEdit->setText(released.toString("MMMM d, yyyy"));
    ui->titleLineEdit->setText(s->getTitle());
    ui->tagsTextEdit->setPlainText(s->tagString());
    ui->sizeLineEdit->setText(QString("%d MB").arg(s->getSize()/BYTES_PER_MEGABYTE));
    ui->seriesLineEdit->setText(s->getSeries());
    QString rating = s->getRating().grade();
    int index = ui->ratingComboBox->findData(rating);
    if (index != -1){
        ui->ratingComboBox->setCurrentIndex(index);
    }
    int resolution = s->getHeight();
    if (resolution > 0){
        ui->resolutionLineEdit->setText(QString::number(resolution));
    }
    QStringList cast = s->getActors();
    for (int i = 0; i < cast.size(); ++i){
        QString name = cast.at(i);
        if (!name.isEmpty()){
            castList.at(i)->setText(QString("<a href=\"%1\">%1</a>").arg(cast.at(i)));
            if (!released.isNull() && released.isValid()){
                emit requestActorBirthday(cast.at(i));
            }
        }
    }
   enableLineEdits(false);
   this->show();
}

void SceneDetailView::receiveActorBirthday(QString name, QDate date){
    bool found = false;
    for(int i = 0; i < 4 && !found; ++i){
        if (castList.at(i)->text().contains(name, Qt::CaseInsensitive)){
            found = true;
            int age = (date.daysTo(QDate::fromString(ui->releasedLineEdit->text(), "MMMM d, yyyy"))/365);
            ageList.at(i)->setText(QString("Age: %1").arg(age));
        }
    }
}

void SceneDetailView::clearDisplay(){
    for(int i = 0; i < 4; ++i){
        castList.at(i)->clear();
        ageList.at(i)->clear();
    }
    ui->titleLineEdit->clear();
    ui->durationLineEdit->clear();
    ui->releasedLineEdit->clear();
    ui->resolutionLineEdit->clear();
    ui->sizeLineEdit->clear();
    ui->tagsTextEdit->clear();
    ui->seriesLineEdit->clear();
    ui->ratingComboBox->clear();

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

void SceneDetailView::on_tb_hide_clicked(){
    this->hide();
}

void SceneDetailView::enableLineEdits(bool readOnly){
    bool enabled = (!readOnly);
    foreach(QLineEdit *line, dataFields){
        line->setReadOnly(readOnly);
    }
    ui->tagsTextEdit->setReadOnly(readOnly);
    ui->tb_save->setEnabled(enabled);
    ui->tb_edit->setEnabled(readOnly);
}

void SceneDetailView::on_tb_save_clicked(){
    current->setTitle(ui->titleLineEdit->text());
    current->setSeries(ui->seriesLineEdit->text());
    current->setLength(QTime::fromString("h:mm:ss"));
    current->setReleased(QDate::fromString("MMMM d, yyyy"));
    current->setHeight(ui->resolutionLineEdit->text().toInt());
    current->setTags(ui->tagsTextEdit->toPlainText().split(",", QString::SkipEmptyParts));
    current->setRating(ui->ratingComboBox->currentText());
    emit saveChanges(current);
    enableLineEdits(true);
}

void SceneDetailView::on_tb_edit_clicked(){
    enableLineEdits(false);
}
