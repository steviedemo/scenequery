#include "profiledialog.h"
#include "ui_profiledialog.h"
#include "Height.h"
#include <QDate>
#include <QInputDialog>
#include <QFileInfo>
#include "filenames.h"
#include "FilePath.h"
ProfileDialog::ProfileDialog(ActorPtr a, QWidget *parent) :
    QDialog(parent), ui(new Ui::ProfileDialog), actor(a){
    ui->setupUi(this);
    setUpFields();
    printDetails(a);
}
ProfileDialog::ProfileDialog(QWidget *parent):
    QDialog(parent), ui(new Ui::ProfileDialog){
    ui->setupUi(this);
    setUpFields();
    QString name = QInputDialog::getText(this, tr("Enter Name"), tr("Enter name of actor to build profile for"));
    if (!name.isNull()){
        emit pd_to_ct_getProfile(name);
    }
}

ProfileDialog::~ProfileDialog(){
    emit closed();
    delete ui;
}

void ProfileDialog::ct_to_pd_receiveProfile(ActorPtr a){
    if (a.isNull()){
        qWarning("Profile Dialog recieved null pointer from curl thread.");
    } else {
        this->actor = a;
        printDetails(actor);
    }
}
void ProfileDialog::db_to_pd_receiveProfileWithID(ActorPtr a){
    if (!a.isNull()){
        this->actor = a;
        emit pd_to_mw_addDisplayItem(a);
    }
}

void ProfileDialog::setUpFields(){
    lineEdits.push_back(ui->labelName);
    lineEdits.push_back(ui->birthCityLineEdit);
    lineEdits.push_back(ui->birthCountryLineEdit);
    lineEdits.push_back(ui->ethnicityLineEdit);
    lineEdits.push_back(ui->eyeColorLineEdit);
    lineEdits.push_back(ui->hairColorLineEdit);
    lineEdits.push_back(ui->headshotLocationLineEdit);
    lineEdits.push_back(ui->heightLineEdit);
    lineEdits.push_back(ui->measurementsLineEdit);
    lineEdits.push_back(ui->weightLineEdit);
    textEdits.push_back(ui->aliasesEdit);
    textEdits.push_back(ui->tattoosEdit);
    textEdits.push_back(ui->piercingsEdit);
    clearDetails();
}

void ProfileDialog::clearDetails(){
    foreach(QLineEdit *le, lineEdits){
        le->clear();
        le->setPlaceholderText("Unknown");
    }
    foreach(QTextEdit *te, textEdits){
        te->clear();
        te->setPlaceholderText("Unknown");
    }
    ui->birthdateDateEdit->setDate(QDate(1988, 1, 1));
}

void ProfileDialog::printDetails(ActorPtr actor){
    ui->labelName->setText(actor->getName());
    ui->aliasesEdit->setText(actor->getAliases());
    ui->birthCityLineEdit->setText(actor->getCity());
    ui->birthCountryLineEdit->setText(actor->getNationality());
    ui->birthdateDateEdit->setDate(actor->getBirthday());
    ui->ethnicityLineEdit->setText(actor->getEthnicity());
    ui->eyeColorLineEdit->setText(actor->getEyeColor());
    ui->hairColorLineEdit->setText(actor->getHairColor());
    ui->headshotLocationLineEdit->setText(actor->getHeadshot().absolutePath());
    Height h = actor->getHeight();
    QString height = QString("%1'%2''").arg(h.getFeet()).arg(h.getInches());
    ui->heightLineEdit->setText(height);
    ui->measurementsLineEdit->setText(actor->getMeasurements());
    ui->piercingsEdit->setText(actor->getPiercings());
    ui->tattoosEdit->setText(actor->getTattoos());
    ui->weightLineEdit->setText(QString::number(actor->getWeight()));
    FilePath photo = a->getHeadshot();

    if (photo.exists() && photo.size() > 200){
        QPixmap profilePhoto(photo.absolutePath());
        ui->profilePhoto->setPixmap(profilePhoto.scaledToHeight(IMAGE_HEIGHT));
    } else {
        QPixmap profilePhoto(DEFAULT_PROFILE_PHOTO);
        ui->profilePhoto->setPixmap(profilePhoto.scaledToHeight(IMAGE_HEIGHT));
    }
}
void ProfileDialog::on_tryAgainButton_clicked(){
    QString filepath = getProfilePhoto(actor->getName());
    QFileInfo info(filepath);
    if (info.exists() && info.size() > 10){
        ui->profilePhoto->setPixmap(QPixmap(filepath).scaledToHeight(IMAGE_HEIGHT));
    }
}
