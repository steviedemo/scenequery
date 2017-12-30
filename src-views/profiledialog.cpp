#include "profiledialog.h"
#include "ui_profiledialog.h"
#include "Height.h"
#include <QDate>
#include <QFileInfo>
#include "filenames.h"
#include "FilePath.h"
ProfileDialog::ProfileDialog(ActorPtr a, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProfileDialog),
    actor(a)
{
    ui->setupUi(this);
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
        QPixmap profilePhoto(":/Icons/blank_profile_photo.png");
        ui->profilePhoto->setPixmap(profilePhoto.scaledToHeight(IMAGE_HEIGHT));
    }
}

ProfileDialog::~ProfileDialog(){
    delete ui;
}

void ProfileDialog::closeEvent(QCloseEvent *event){
    emit closed();
    event->accept();
}

void ProfileDialog::on_tryAgainButton_clicked(){
    QString filepath = getProfilePhoto(actor->getName());
    QFileInfo info(filepath);
    if (info.exists() && info.size() > 10){
        ui->profilePhoto->setPixmap(QPixmap(filepath).scaledToHeight(IMAGE_HEIGHT));
    }
}
