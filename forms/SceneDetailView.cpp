#include "SceneDetailView.h"
#include "ui_SceneDetailView.h"
#include <QList>
#include <QInputDialog>
#include <QtGui>
#include <QLabel>
#include <QListIterator>
#include <QMessageBox>
#include "SceneParser.h"
#include "Scene.h"
#include "genericfunctions.h"
#include "Rating.h"
SceneDetailView::SceneDetailView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SceneDetailView)
{
    ui->setupUi(this);
    changed = false;
    this->castList      << ui->actor1label  << ui->actor2label  << ui->actor3label  << ui->actor4label;
    this->ageList       << ui->age1label    << ui->age2label    << ui->age3label    << ui->age4label;
    this->ageLabelList  << ui->labelAge_1   << ui->labelAge_2   << ui->labelAge_3   << ui->labelAge_4;
    this->dataFields    << ui->titleLineEdit;
    this->dataFields    << ui->filenameLineEdit;
    this->dataFields    << ui->companyLineEdit  << ui->seriesLineEdit;
    this->dataFields    << ui->durationLineEdit << ui->sizeLineEdit     << ui->resolutionLineEdit;
    this->dataFields    << ui->addedLineEdit    << ui->openedLineEdit   << ui->releasedLineEdit;
    for(int i = 0; i < 4; ++i){
        QLabel *label = castList.at(i);
        label->setMouseTracking(true);
        label->setOpenExternalLinks(false);
        label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        connect(castList.at(i), SIGNAL(linkActivated(QString)), this, SLOT(actorLinkClicked(QString)));
    }
    connect(ui->tb_add_actor, SIGNAL(pressed()), this, SLOT(addActor()));
    connect(ui->pb_play, SIGNAL(pressed()), this, SLOT(playCurrentVideo()));
    connect(ui->pb_hide, SIGNAL(pressed()), this, SLOT(hide()));
    QStringList ratingValues = getRatingList();
    foreach(QString rating, ratingValues){
        ui->ratingComboBox->addItem(rating, rating);
    }
    //enableLineEdits(true);
}

SceneDetailView::~SceneDetailView(){
    delete ui;
}

void SceneDetailView::loadScene(ScenePtr s){
    if (!current.isNull() && changed){
        QMessageBox b(QMessageBox::Question, tr("Unsaved Changes"), tr("Save changes before leaving this page?"),QMessageBox::Save | QMessageBox::No, this, Qt::WindowStaysOnTopHint);
        if (b.exec() == QMessageBox::Save){
            emit saveChanges(current);
        }
    }
    this->current = ScenePtr(s.data());
    ui->titleLineEdit->setText(s->getTitle());
    ui->companyLineEdit->setText(s->getCompany());
    ui->seriesLineEdit->setText(s->getSeries());
    QTime duration = s->getLength();
    ui->durationLineEdit->setText(duration.toString("h:mm:ss"));
    QDate released = s->getReleased();
    QDate added = s->getAdded();
    QDate opened = s->getOpened();
    ui->releasedLineEdit->setText(released.toString("MMMM d, yyyy"));
    ui->addedLineEdit->setText(added.toString("MMMM d, yyyy"));
    ui->openedLineEdit->setText(opened.toString("MMMM d, yyyy"));
    ui->filenameLineEdit->setText(s->getFullpath());

    ui->tagsTextEdit->setPlainText(s->tagString());
    QString rating = s->getRating().grade();
    int index = ui->ratingComboBox->findData(rating);
    if (index != -1){
        ui->ratingComboBox->setCurrentIndex(index);
    }
    ui->sizeLineEdit->setText(QString("%1 MB").arg(s->getSize()/BYTES_PER_MEGABYTE));
    int resolution = s->getHeight();
    if (resolution > 0){
        ui->resolutionLineEdit->setText(QString::number(resolution));
    }
    QStringList cast = s->getActors();
    for (int i = 0; i < cast.size(); ++i){
        QString name = cast.at(i);
        ageList.at(i)->clear();
        if (!name.isEmpty()){
            castList.at(i)->setText(QString("<a href=\"%1\"><font style=\"color:white\">%1</font></a>").arg(cast.at(i)));
            if (!released.isNull() && released.isValid()){
                emit requestActorBirthday(cast.at(i));
            }
        } else {
            castList.at(i)->clear();
            ageLabelList.at(i)->clear();
        }
    }
   //enableLineEdits(true);
   foreach(QLineEdit *line, dataFields){
       if (line->text().isEmpty()){
           line->setReadOnly(false);
       }
   }
   changed = false;
   this->show();
}

void SceneDetailView::on_pb_reparse_clicked()
{
    this->rescanScene();
}

void SceneDetailView::rescanScene(){
    if (!current.isNull()){
        QPair<QString,QString> sceneFile = current->getFile();
        qDebug("Reparsing Scene...");
        sceneParser p(sceneFile);
        p.parse();
        this->current = ScenePtr(new Scene(p));
        this->loadScene(current);
        qDebug("Scene Parsed!");
    } else {
        qWarning("Unable to re-parse Null Scene");
    }
}

void SceneDetailView::playCurrentVideo(){
    emit playVideo(current->getFullpath());
}

void SceneDetailView::addActor(){
    QString text = QInputDialog::getText(this, tr("Enter Actor Name"), tr("Add an Actor"));
    if (!text.isEmpty()){
        foreach(QLabel *nameLabel, this->castList){
            if (nameLabel->text() == text){
                break;  // Don't add the same name twice.
            } else if (nameLabel->text().isEmpty()){
                current->addActor(text);
                nameLabel->setText(QString("<a href=\"%1\"><font style=\"color:white\">%1</font></a>").arg(text));
                ui->pb_save->setEnabled(true);
                changed = true;
            }
        }
    }
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
    ui->ratingComboBox->setCurrentIndex(-1);
    changed = false;
}

void SceneDetailView::actorLinkClicked(QString name){
    if (!name.isEmpty()){
        emit showActor(name);
    }
}

void SceneDetailView::enableLineEdits(bool readOnly){
    foreach(QLineEdit *line, dataFields){
        line->setReadOnly(readOnly);
    }
    ui->tagsTextEdit->setReadOnly(readOnly);
    ui->tb_edit->setEnabled(readOnly);
}

void SceneDetailView::on_pb_save_clicked(){
    if (!current.isNull()){
        QString text = ui->titleLineEdit->text();
        if (valid(text) && text != current->getTitle()){
            current->setTitle(text);
        }
        text = ui->seriesLineEdit->text();
        if (valid(text) && text != current->getSeries()){
            current->setSeries(text);
        }
        text = ui->durationLineEdit->text();
        if (valid(text) && text != current->getLength().toString("h::mm:ss")){
            current->setLength(QTime::fromString(text, "h:mm:ss"));
        }
        text = ui->releasedLineEdit->text();
        if (valid(text) && text != current->getReleased().toString("MMMM d, yyyy")){
            current->setReleased(QDate::fromString(text, "MMMM d, yyyy"));
        }
        text = ui->tagsTextEdit->toPlainText();
        if (valid(text) && text != current->tagString()){
            current->setTags(text.split(',', QString::SkipEmptyParts));
        }
        if (ui->ratingComboBox->currentIndex() != -1 && !ui->ratingComboBox->currentText().isEmpty()){
            this->current->setRating(ui->ratingComboBox->currentText());
        }
        emit saveChanges(current);
        changed = false;
        //enableLineEdits(true);
    } else {
        qWarning("Error Setting current scene details");
    }
}

