#include "ActorProfileView.h"
#include "filenames.h"
#include "Actor.h"
#include <unistd.h>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include "imageeditor.h"
ActorProfileView::ActorProfileView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActorProfileView){
    ui->setupUi(this);
    this->setupFields();
    this->sc_downloadCurrentProfile = new QShortcut(QKeySequence("Ctrl+d"), this);
    this->sc_saveChangesToActor = new QShortcut(QKeySequence("Ctrl+s"), this);
    this->sc_chooseNewPhoto = new QShortcut(QKeySequence("Ctrl+n"), this);
    this->sc_hideProfile = new QShortcut(QKeySequence("Esc"), this);
    connect(sc_downloadCurrentProfile,  SIGNAL(activated()), this, SLOT(on_updateFromWeb_clicked()));
    connect(sc_saveChangesToActor,      SIGNAL(activated()), this, SLOT(on_saveProfile_clicked()));
    connect(sc_hideProfile,             SIGNAL(activated()), this, SLOT(on_closeProfile_clicked()));
    connect(sc_chooseNewPhoto,          SIGNAL(activated()), this, SLOT(on_selectNewPhoto_clicked()));
}

ActorProfileView::~ActorProfileView(){
    delete sc_downloadCurrentProfile;
    delete sc_saveChangesToActor;
    delete sc_chooseNewPhoto;
    delete sc_hideProfile;
    delete ui;
}

void ActorProfileView::setupFields(){
    lineEdits.push_back(ui->scenesLineEdit);
    lineEdits.push_back(ui->ethnicityLineEdit);
    lineEdits.push_back(ui->birthCityLineEdit);
    lineEdits.push_back(ui->ageLineEdit);
    lineEdits.push_back(ui->measurementsLineEdit);
    lineEdits.push_back(ui->nationalityLineEdit);
    lineEdits.push_back(ui->hairColorLineEdit);
    lineEdits.push_back(ui->eyeColorLineEdit);
    lineEdits.push_back(ui->heightLineEdit);
    lineEdits.push_back(ui->weightLineEdit);
    textEdits.push_back(ui->aliasesTextEdit);
    textEdits.push_back(ui->piercingsTextEdit);
    textEdits.push_back(ui->tattoosTextEdit);
    foreach(QLineEdit *le, lineEdits){
        le->setPlaceholderText("Unknown");
    }
    foreach(QTextEdit *te, textEdits){
        te->setPlaceholderText("Unknown");
    }
    ui->birthDateDateEdit->setDate(QDate(1988, 1, 1));
}
void ActorProfileView::clearFields(){
    foreach(QLineEdit *le, lineEdits){
        le->clear();
    }
    foreach(QTextEdit *te, textEdits){
        te->clear();
    }
    ui->birthDateDateEdit->clear();
}

void ActorProfileView::acceptSceneCount(int count){
    ui->scenesLineEdit->setText(QString::number(count));
}

void ActorProfileView::on_deletePhoto_clicked(){
    if (!current->usingDefaultPhoto()){
        QString filename = getHeadshotName(current->getName());
        QFile file(filename);
        if (file.exists()){
            file.remove();
            current->setDefaultHeadshot();
            ui->profilePhoto->setPixmap(QPixmap(DEFAULT_PROFILE_PHOTO).scaledToHeight(IMAGE_HEIGHT));
        }
    }
}

void ActorProfileView::on_clearFields_clicked(){
    clearFields();
}

void ActorProfileView::on_selectNewPhoto_clicked(){
    QString saveFile = getHeadshotName(this->current->getName());
    QString sourceFile("");
    sourceFile = QFileDialog::getOpenFileName(this, tr("Select Image"), QString(), "*.jpg;*.jpeg;*.png");
    qDebug("Verifying existence of selected Photo (%s)...", qPrintable(sourceFile));
    if (!sourceFile.isEmpty()){
        if (QFileInfo(sourceFile).exists()){
            qDebug("Opening Editor with %s", qPrintable(sourceFile));
            if (!sourceFile.isEmpty()){
                this->editor = QSharedPointer<ImageEditor>(new ImageEditor(sourceFile, saveFile));
                connect(editor.data(), SIGNAL(saved()), this, SLOT(reloadProfilePhoto()));
                editor->show();
            }
        } else {
            QMessageBox::warning(this, tr("Error"), QString("Unable to open %1").arg(sourceFile), QMessageBox::Ok);
        }
    }
}

void ActorProfileView::reloadProfilePhoto(){
    if (!this->isHidden() && !this->current.isNull()){
        ui->profilePhoto->setPixmap(QPixmap(current->getHeadshot()).scaledToHeight(IMAGE_HEIGHT));
        current->updateQStandardItem();
    }
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
    QImage headshot = scaleImage(a->getHeadshot(), IMAGE_HEIGHT, Qt::SmoothTransformation);
    ui->profilePhoto->setPixmap(QPixmap::fromImage(headshot));
    /// Request The scene count in a few seconds, once the scene filtering has been performed.
    QTimer::singleShot(3, this, SLOT(onTimeout()));
}

void ActorProfileView::onTimeout(){
    emit requestSceneCount();
}

void ActorProfileView::on_deleteActor_clicked(){
    emit deleteCurrent();
    emit deleteActor(current);
}
void ActorProfileView::on_downloadPhoto_clicked(){
    emit downloadPhoto(current);
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
        current->updateQStandardItem();
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

void ActorProfileView::on_editProfile_clicked(){
    if (ui->birthDateDateEdit->isHidden()){
        ui->birthDateDateEdit->show();
        ui->birthDateLabel->show();
        ui->ageLabel->show();
        ui->ageLineEdit->show();
    }
}
