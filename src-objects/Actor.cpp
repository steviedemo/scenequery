#include "Actor.h"
#include "Scene.h"
#include "SceneList.h"
#include "Query.h"
#include "curlTool.h"
#include "filenames.h"
#include "SQL.h"
#include <QRegularExpression>
#include <QStringList>
#include <QPixmap>
#include "curlTool.h"

void Actor::fromRecord(pqxx::result::const_iterator i){
    try{

        if (!i.at("id").is_null())         {   setID((qint64)i["id"].as<long long unsigned int>());                }
        if (!i.at("name").is_null())       {   this->name = QString::fromStdString(i["name"].as<std::string>());   }
        if (!i.at("aliases").is_null())    {   this->bio.setAliases(QString::fromStdString(i["aliases"].as<std::string>())); }
        if (!i.at("birthday").is_null()){
            QString temp = QString::fromStdString(i["birthday"].as<std::string>());
            this->bio.setBirthday(QDate::fromString(temp, "yyyy-MM-dd"));
        }
        if (!i.at("city").is_null())       {   this->bio.setCity(QString::fromStdString(i["city"].as<std::string>()));      }
        if (!i.at("country").is_null())    {   this->bio.setNationality(QString::fromStdString(i["country"].as<std::string>()));   }
        if (!i.at("ethnicity").is_null())  {   this->bio.setEthnicity(QString::fromStdString(i["ethnicity"].as<std::string>())); }
        if (!i.at("height").is_null())     {   this->bio.setHeight(Height(i["height"].as<int>()));   /* convert from cm */    }
        if (!i.at("weight").is_null())     {   this->bio.setWeight(i["weight"].as<int>());                                    }
        if (!i.at("measurements").is_null()){  this->bio.setMeasurements(QString::fromStdString(i["measurements"].as<std::string>()));  }
        if (!i.at("hair").is_null())       {   this->bio.setHairColor(QString::fromStdString(i["hair"].as<std::string>()));       }
        if (!i.at("eyes").is_null())       {   this->bio.setEyeColor(QString::fromStdString(i["eyes"].as<std::string>()));       }
        if (!i.at("tattoos").is_null())    {   this->bio.setTattoos(QString::fromStdString(i["tattoos"].as<std::string>()));    }
        if (!i.at("piercings").is_null())  {   this->bio.setPiercings(QString::fromStdString(i["piercings"].as<std::string>()));  }
        if (!name.isEmpty()){
            this->photoPath = getProfilePhoto(name);
        }
    } catch(std::exception &e){
        qWarning("Error Caught while making Actor from record: %s", e.what());
    }
    setup();
}

Actor Actor::operator =(Actor &obj){
    this->bio = obj.bio;
    this->name = obj.name;
    this->dataUsage = obj.dataUsage;
    this->photoPath = obj.photoPath;
    return *this;
}


bool Actor::inDatabase(){
    bool found = false;
    sqlConnection sql(QString("SELECT FROM actors WHERE name = %1").arg(name));
    if (sql.execute()){
        found = sql.foundMatch();
    }
    return found;
}

void Actor::deleteHeadshot(){
    if (!photoPath == DEFAULT_PROFILE_PHOTO){
        qDebug("Removing '%s'...", qPrintable(photoPath));
        QFile file(photoPath);
        if (!file.remove()){
            qWarning("Error Removing File '%s'", qPrintable(photoPath));
        }
        file = QFile(getHeadshotThumbnailName(name));
        if (file.exists()){
            qDebug("Removing '%s'...", qPrintable(file.fileName()));
            if (!file.remove()){
                qWarning("Error Removing file '%s'", qPrintable(file.fileName()));
            }
        }
        setDefaultHeadshot();
    }
}
void Actor::setDefaultHeadshot(){
    this->photoPath = DEFAULT_PROFILE_PHOTO;
    this->profilePhoto = QVariant(QPixmap(DEFAULT_PROFILE_PHOTO).scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT));
    this->itemPhoto->setData(QVariant(profilePhoto), Qt::DecorationRole);
}

QList<QStandardItem *> Actor::buildQStandardItem(){
    //qDebug("Creating Display item for %s", qPrintable(name));
    this->itemName      = new QStandardItem(name);
    this->itemHair      = new QStandardItem(bio.getHairColor());
    this->itemEthnicity = new QStandardItem(bio.getEthnicity());
    this->itemSceneCount= new QStandardItem(QString::number(sceneCount));
    this->itemPhoto     = new QStandardItem();
    this->itemBioSize   = new QStandardItem();
    this->itemAge       = new QStandardItem();
    this->itemHeight    = new QStandardItem();
    this->itemWeight    = new QStandardItem();
    this->itemTattoos   = new QStandardItem();
    this->itemPiercings = new QStandardItem();
    QDate birthday = bio.getBirthday();
    if (birthday.isValid()){
        int years = birthday.daysTo(QDate::currentDate())/365;
        itemAge->setText(QString::number(years));
    }
    if (bio.getHeight().isValid()){
        Height h = bio.getHeight();
        itemHeight->setText(QString("%1'%2''").arg(h.getFeet()).arg(h.getInches()));
    }
    if (bio.getWeight() > 0){
        itemWeight->setText(QString("%1 lbs").arg(bio.getWeight()));
    }
    QString tatts = bio.getTattoos();
    if (!tatts.isEmpty() && !tatts.contains("None")){
        itemTattoos->setText("Yes");
    } else {
        itemTattoos->setText("No");
    }
    QString rings = bio.getPiercings();
    if (!rings.isEmpty() && !rings.contains("None")){
        itemPiercings->setText("Yes");
    } else {
        itemPiercings->setText("No");
    }
/*
    QString bioSize = QString("%1").arg(size(), 2, 10, QChar('0'));
    QString scenes = QString("%1").arg(sceneCount, 2, 10, QChar('0'));
    this->itemBioSize->setText(bioSize);
    this->itemSceneCount->setText(scenes);
    this->itemSceneCount->setTextAlignment(Qt::AlignLeft);
    this->itemSceneCount->setTextAlignment(Qt::AlignCenter);
*/
    if (this->photoPath.isEmpty()){
        this->photoPath = getProfilePhoto(name);
    }
    QImage scaledImage = scaleImage(photoPath, 30);
    //QImage scaledImage(getHeadshotThumbnail(name));
    this->itemPhoto->setData(QVariant(scaledImage), Qt::DecorationRole);
    //this->itemPhoto->setData(QVariant(QPixmap(photo).scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT)), Qt::DecorationRole);
    row << itemPhoto << itemName << itemHair << itemEthnicity << itemAge << itemHeight << itemWeight << itemTattoos << itemPiercings;
    this->displayItemCreated = true;
    return row;
}

void Actor::updateQStandardItem(){
    this->photoPath = getProfilePhoto(name);
    this->itemSceneCount->setText(QString::number(sceneList.size()));
    if (itemHair->text().isEmpty()){
        QString hair = bio.getHairColor();
        itemHair->setText(hair);
    }
    if (itemEthnicity->text().isEmpty()){
        QString ethnicity = bio.getEthnicity();
        itemEthnicity->setText(ethnicity);
    }
    this->photoPath = getProfilePhoto(name);
    QImage scaledImage = scaleImage(photoPath, 30);
    this->itemPhoto->setData(QVariant(scaledImage), Qt::DecorationRole);

    this->itemBioSize->setText(QString("%1").arg(size(), 2, 10, QChar('0')));
    this->itemSceneCount->setText(QString("%1").arg(sceneCount, 2, 10, QChar('0')));
}

Query Actor::toQuery() const{
    Query q;
    q.setTable("actors");
    q.add("NAME",           name);
    q.add("ALIASES",        bio.getAliases());
    if (!bio.getBirthday().isNull() && bio.birthdate.isValid()){
        q.add("BIRTHDAY",   bio.getBirthday());
    }
    q.add("CITY",           bio.getCity());
    q.add("COUNTRY",        bio.getNationality());
    q.add("ETHNICITY",      bio.getEthnicity());
    if (bio.getHeight().nonZero()){
        q.add("HEIGHT",     bio.getHeight());
    }
    q.add("WEIGHT",         bio.getWeight());
    q.add("MEASUREMENTS",   bio.getMeasurements());
    q.add("HAIR",           bio.getHairColor());
    q.add("EYES",           bio.getEyeColor());
    q.add("TATTOOS",        bio.getTattoos());
    q.add("PIERCINGS",      bio.getPiercings());
    q.addCriteria("ID", QString::number(this->ID));
    //qDebug("Actor Query Object Successfully Created");
    return q;
}
