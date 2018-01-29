#include "SceneDetailView.h"
#include "ui_SceneDetailView.h"
#include <QList>
#include <QInputDialog>
#include <QtGui>
#include <QLabel>
#include <QListIterator>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include "SceneParser.h"
#include "Scene.h"
#include "genericfunctions.h"
#include "Rating.h"
SceneDetailView::SceneDetailView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SceneDetailView), currentSceneID(-1)
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
    connect(ui->pb_play, &QPushButton::pressed, [=]{    emit playVideo(this->currentSceneID);   });
    connect(ui->pb_hide, SIGNAL(pressed()),      this, SLOT(hide()));
    connect(ui->pb_reparse,  SIGNAL(clicked()),  this, SLOT(rescanScene()));
    QStringList ratingValues = getRatingList();
    foreach(QString rating, ratingValues){
        ui->ratingComboBox->addItem(rating, rating);
    }
    //enableLineEdits(true);
}

SceneDetailView::~SceneDetailView(){
    delete ui;
}

void SceneDetailView::hideDetailView(){
    this->clearDisplay();
    this->hide();
}
void SceneDetailView::showDetailView(ScenePtr s){
    this->show();
    this->loadScene(s);
}

void SceneDetailView::updateDetailView(ScenePtr s){
    if (!this->isHidden()){
        loadScene(s);
    }
}

void SceneDetailView::loadScene(ScenePtr s){
    try{
        if (!current.isNull() && changed){
            QMessageBox b(QMessageBox::Question, tr("Unsaved Changes"), tr("Save changes before leaving this page?"),QMessageBox::Save | QMessageBox::No, this, Qt::WindowStaysOnTopHint);
            if (b.exec() == QMessageBox::Save){
                emit saveChanges(current);
            }
        }
    } catch (std::exception &e){
        qWarning("Caught Exception While Displaying menu: %s", e.what());
    }
    try{
        this->current = s;
    } catch (std::bad_alloc &e){
        qWarning("Bad Alloc Exception Caught while re-assigning 'current' scene object: %s", e.what());
        return;
    } catch (std::exception &e){
        qWarning("Exception Caught while re-assigning 'current' scene object: %s", e.what());
        return;
    }
    QDate released = s->getReleased();
    QDate added = s->getAdded();
    QDate opened = s->getOpened();
    try{
        this->currentSceneID = s->getID();
        ui->titleLineEdit->setText(s->getTitle());
        ui->companyLineEdit->setText(s->getCompany());
        ui->seriesLineEdit->setText(s->getSeries());
        QTime duration = s->getLength();
        ui->durationLineEdit->setText(duration.toString("h:mm:ss"));
        ui->releasedLineEdit->setText(released.toString("MMMM d, yyyy"));
        ui->addedLineEdit->setText(added.toString("MMMM d, yyyy"));
        ui->openedLineEdit->setText(opened.toString("MMMM d, yyyy"));
        ui->filenameLineEdit->setText(s->getFullpath());
        ui->tagsTextEdit->setPlainText(s->tagString());
    } catch (std::bad_alloc &e){
        qWarning("Bad Alloc Exception Caught while Setting fields: %s", e.what());
        return;
    } catch (std::exception &e){
        qWarning("Exception Caught while Setting fields : %s", e.what());
        return;
    }
    try{
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
        if (!cast.isEmpty()){
            for (int i = 0; i < ageList.size(); ++i){
                castList.at(i)->clear();
                ageList.at(i)->clear();
                ageLabelList.at(i)->clear();
                if (i < cast.size()){
                    QString name = cast.at(i);
                    if (!name.isEmpty()){
                        castList.at(i)->setText(QString("<a href=\"%1\"><font style=\"color:white\">%1</font></a>").arg(cast.at(i)));
                        if (!released.isNull() && released.isValid() && vault->contains(name)){
                            QDate birthday = vault->getActor(name)->getBirthday();
                            if (!birthday.isNull() && birthday.isValid()){
                                int age = (birthday.daysTo(released)/365);
                                ageList.at(i)->setText(QString::number(age));
                                ageLabelList.at(i)->setText("Age:");
                            }
                        }
                    }
                }
            }
        }
    } catch (std::bad_alloc &e){
        qWarning("Exception Caught while setting labels: %s", e.what());
        return;
    } catch (std::exception &e){
        qWarning("Exception Caught while setting labels: %s", e.what());
        return;
    }
   changed = false;
   this->show();
}

void SceneDetailView::rescanScene(){
    if (!current.isNull()){
        qDebug("Reparsing Scene...");
        this->current->reparse();
        this->loadScene(current);
        qDebug("Scene Parsed!");
    } else {
        qWarning("Unable to re-parse Null Scene");
    }
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

//void SceneDetailView::receiveActorBirthday(QString name, QDate date){
//    bool found = false;
//    for(int i = 0; i < 4 && !found; ++i){
//        if (castList.at(i)->text().contains(name, Qt::CaseInsensitive)){
//            found = true;
//            int age = (date.daysTo(QDate::fromString(ui->releasedLineEdit->text(), "MMMM d, yyyy"))/365);
//            ageList.at(i)->setText(QString("Age: %1").arg(age));
//        }
//    }
//}

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
        if (vault->contains(name)){
            emit showActor(vault->getActor(name));
        }
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

