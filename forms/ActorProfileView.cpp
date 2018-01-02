#include "ActorProfileView.h"
#include "ui_actorprofileview.h"
#include "Actor.h"
#include <unistd.h>
#include <QTimer>
ActorProfileView::ActorProfileView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActorProfileView){
    ui->setupUi(this);
}

ActorProfileView::~ActorProfileView()
{
    delete ui;
}
void ActorProfileView::clearFields(){
    ui->scenesLineEdit->setText("");
    ui->ethnicityLineEdit->setText("");
    ui->aliasesTextEdit->setText("");
    ui->birthCityLineEdit->setText("");
    ui->ageLineEdit->setText("");
    ui->measurementsLineEdit->setText("");
    ui->nationalityLineEdit->setText("");
    ui->hairColorLineEdit->setText("");
    ui->eyeColorLineEdit->setText("");
    ui->heightLineEdit->setText("");
    ui->weightLineEdit->setText("");
    ui->piercingsTextEdit->setText("");
    ui->tattoosTextEdit->setText("");
}

void ActorProfileView::acceptSceneCount(int count){
    ui->scenesLineEdit->setText(QString::number(count));
}

void ActorProfileView::loadActorProfile(ActorPtr a){
    this->current = a;
    clearFields();
    this->show();
    ui->label_name->setText(a->getName());
    ui->saveProfile->setDisabled(true);
    ui->reloadFromDb->setDisabled(true);
    Biography bio = a->getBio();
    /// Set Basic Fields
    ui->ethnicityLineEdit->setText(bio.getEthnicity());
    ui->eyeColorLineEdit->setText(bio.getEyeColor());
    ui->hairColorLineEdit->setText(bio.getHairColor());
    ui->measurementsLineEdit->setText(bio.getMeasurements());
    // Put together the birthplace label
    ui->birthCityLineEdit->setText(bio.getCity());
    ui->nationalityLineEdit->setText(bio.getNationality());
    // Put together the height label
    Height height = bio.getHeight();
    if (height.nonZero()){
        ui->heightLineEdit->setText(height.toString());
    } else {
        ui->heightLineEdit->setText("");
    }
    int weight = bio.getWeight();
    if (weight > 0){
        QString temp = QString::number(weight) + " lbs";
        ui->weightLineEdit->setText(temp);
    } else {
        ui->weightLineEdit->setText("");
    }
    // Set Birthday & Age
    QDate birthday = bio.getBirthday();
    if (!birthday.isNull() && birthday.isValid()){
        ui->birthDateLabel->show();
        ui->birthDateDateEdit->show();
        ui->birthDateDateEdit->setDate(birthday);
        int age = (birthday.daysTo(QDate::currentDate()))/365;
        ui->ageLabel->show();
        ui->ageLineEdit->show();
        ui->ageLineEdit->setText(QString("%1 years old").arg(age));

    } else {
        ui->birthDateLabel->hide();
        ui->birthDateDateEdit->hide();
        ui->ageLabel->hide();
        ui->ageLineEdit->hide();
    }
    /// Set the content of the multi-line text boxes.
    ui->aliasesTextEdit->setText(bio.getAliases());
    ui->piercingsTextEdit->setText(bio.getPiercings());
    ui->tattoosTextEdit->setText(bio.getTattoos());
    ui->profilePhoto->setPixmap(QPixmap(a->getHeadshotPath()).scaledToHeight(IMAGE_HEIGHT));
    /// Request The scene count in a few seconds, once the scene filtering has been performed.
    QTimer::singleShot(3, this, SLOT(onTimeout()));
}

void ActorProfileView::onTimeout(){
    emit requestSceneCount();
}

/** \brief Save any changes made to the currently displayed profile */
void ActorProfileView::on_saveProfile_clicked(){
    ActorPtr a = current;
    if (!current.isNull()){
        current->setAliases(ui->aliasesTextEdit->toPlainText());
        current->setPiercings(ui->piercingsTextEdit->toPlainText());
        current->setEthnicity(ui->ethnicityLineEdit->text());
        current->setEyes(ui->eyeColorLineEdit->text());
        current->setHair(ui->hairColorLineEdit->text());
        current->setMeasurements(ui->measurementsLineEdit->text());
        current->setTattoos(ui->tattoosTextEdit->toPlainText());
        // Read in Birthdate
        QDate birthday = ui->birthDateDateEdit->date();
        if (birthday != QDate(1988, 1, 1)){
            current->setBirthday(birthday);
        }
        // Read in height
        QRegularExpression heightRx("([0-9])\\'([1]?[0-9])\\\"");
        QRegularExpressionMatch heightMatch = heightRx.match(ui->heightLineEdit->text());
        if (heightMatch.hasMatch()){
            bool feetOk = false, inchesOk = false;
            int feet = heightMatch.captured(1).toInt(&feetOk);
            int inches = heightMatch.captured(2).toInt(&inchesOk);
            if (feetOk && inchesOk){
                Height h(feet, inches);
                current->setHeight(h);
            }
        }
        // Read in weight
        QString weightString = ui->weightLineEdit->text();
        if (!weightString.isEmpty() && !weightString.isNull() && weightString != "0"){
            bool ok = false;
            int w = weightString.toInt(&ok);
            if (ok){    current->setWeight(w); }
        }
        current->setCity(ui->birthCityLineEdit->text());
        current->setNationality(ui->nationalityLineEdit->text());
        emit saveToDatabase(a);
    }
}


void ActorProfileView::on_updateFromWeb_clicked(){
    emit updateFromWeb(current);
}

void ActorProfileView::on_closeProfile_clicked(){
    emit hidden();
    this->hide();
}

void ActorProfileView::on_reloadFromDb_clicked(){
    emit clearChanges();
}
void ActorProfileView::on_birthDateDateEdit_userDateChanged(const QDate &/*date*/){   setResetAndSaveButtons(true);   }
void ActorProfileView::on_hairColorLineEdit_textChanged(const QString &/*arg1*/)  {   setResetAndSaveButtons(true);   }
void ActorProfileView::on_ethnicityLineEdit_textChanged(const QString &)          {   setResetAndSaveButtons(true);   }
void ActorProfileView::on_nationalityLineEdit_textEdited(const QString &)         {   setResetAndSaveButtons(true);   }
void ActorProfileView::on_heightLineEdit_textEdited(const QString &)              {   setResetAndSaveButtons(true);   }
void ActorProfileView::on_weightLineEdit_textEdited(const QString &)              {   setResetAndSaveButtons(true);   }
void ActorProfileView::on_eyeColorLineEdit_textEdited(const QString &)            {   setResetAndSaveButtons(true);   }
void ActorProfileView::on_measurementsLineEdit_textEdited(const QString &)        {   setResetAndSaveButtons(true);   }
void ActorProfileView::on_aliasesTextEdit_textChanged()                           {   setResetAndSaveButtons(true);   }
void ActorProfileView::on_piercingsTextEdit_textChanged()                         {   setResetAndSaveButtons(true);   }
void ActorProfileView::on_tattoosTextEdit_textChanged()                           {   setResetAndSaveButtons(true);   }
void ActorProfileView::setResetAndSaveButtons(bool enabled){
    ui->reloadFromDb->setEnabled(enabled);
    ui->saveProfile->setEnabled(enabled);
}
