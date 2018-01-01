#include "ActorProfileView.h"
#include "ui_actorprofileview.h"
#include "Actor.h"
ActorProfileView::ActorProfileView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActorProfileView){
    ui->setupUi(this);
}

ActorProfileView::~ActorProfileView()
{
    delete ui;
}

void ActorProfileView::loadActorProfile(ActorPtr a){
    this->current = a;
    this->show();

    ui->label_name->setText(a->getName());
    ui->saveProfile->setDisabled(true);
    ui->reloadFromDb->setDisabled(true);
    Biography bio = a->getBio();
    ui->ethnicityLineEdit->setText(bio.getEthnicity());
    ui->eyeColorLineEdit->setText(bio.getEyeColor());
    ui->hairColorLineEdit->setText(bio.getHairColor());
    ui->measurementsLineEdit->setText(bio.getMeasurements());
    QString city = bio.getCity();
    QString country = bio.getNationality();
    /// Set the birthplace
    QString birthplace("");
    if (!city.isEmpty() && !country.isEmpty()){
        birthplace = QString("%1, %2").arg(city).arg(country);
    } else if (!country.isEmpty()) {
        birthplace = country;
    } else {
        birthplace = "Unknown";
    }
    Height height = bio.getHeight();
    if (height.nonZero()){
        ui->heightLineEdit->setText(height.toString());
    }
    int weight = bio.getWeight();
    if (weight > 0){
        QString temp = QString::number(weight) + " lbs";
        ui->weightLineEdit->setText(temp);
    }
    QString age("");
    QDate birthday = bio.getBirthday();
    if (!birthday.isNull() && birthday.isValid()){
        ui->birthDateDateEdit->setDate(birthday);
    }
    ui->nationalityLineEdit->setText(birthplace);
    /// Set the content of the multi-line text boxes.
    ui->aliasesTextEdit->setText(bio.getAliases());
    ui->piercingsTextEdit->setText(bio.getPiercings());
    ui->tattoosTextEdit->setText(bio.getTattoos());
    ui->profilePhoto->setPixmap(QPixmap(a->getHeadshotPath()).scaledToHeight(IMAGE_HEIGHT));
    //sceneProxyModel->setFilterActor(a->getName());
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
        // Read in Birthplace
        QRegularExpression birthplaceRx("([\\s\\'A-Za-z]+),\\s*(.+)");
        QRegularExpressionMatch bpMatch = birthplaceRx.match(ui->nationalityLineEdit->text());
        if (bpMatch.hasMatch()){
            current->setCity(bpMatch.captured(1));
            current->setNationality(bpMatch.captured(2));
        } else {
            current->setNationality(ui->nationalityLineEdit->text());
        }
        emit saveToDatabase(a);
    }
}


void ActorProfileView::on_updateFromWeb_clicked(){
    emit updateFromWeb(current);
}

void ActorProfileView::on_closeProfile_clicked(){
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
