#include "Actor.h"
#include "Query.h"
#include "curlTool.h"
#include "filenames.h"
#include "sql.h"
#include <QRegularExpression>
#include <QStringList>
#include <QPixmap>
#include "curlTool.h"
Actor::Actor(QString name):
    Entry(), name(name){
    this->dataUsage = 0.0;
    this->headshot = FilePath(getProfilePhoto(name));
    setup();
}
Actor::Actor(const Actor &a):
    Entry(){
    this->bio = a.bio;
    this->name = a.name;
    this->dataUsage = a.dataUsage;
    this->headshot = a.headshot;
    setup();
}

Actor::Actor(QString actorName, Biography bio, QString headshot):
    Entry(), name(actorName), bio(bio), dataUsage(0.0), headshot(headshot){
    setup();
}

Actor::Actor(pqxx::result::const_iterator &i):Entry(){
    this->fromRecord(i);
}

void Actor::fromRecord(pqxx::result::const_iterator &i){
    try{
        if (!i["id"].is_null())         {   setID((qint64)i["id"].as<long long unsigned int>());                }
        if (!i["name"].is_null())       {   this->name = QString::fromStdString(i["name"].as<std::string>());   }
        if (!i["aliases"].is_null())    {   this->bio.setAliases(QString::fromStdString(i["aliases"].as<std::string>())); }
        if (!i["birthday"].is_null()){
            QString temp = QString::fromStdString(i["birthday"].as<std::string>());
            this->bio.setBirthday(QDate::fromString(temp, "yyyy-MM-dd"));
        }
        if (!i["city"].is_null())       {   this->bio.setCity(QString::fromStdString(i["city"].as<std::string>()));      }
        if (!i["country"].is_null())    {   this->bio.setNationality(QString::fromStdString(i["country"].as<std::string>()));   }
        if (!i["ethnicity"].is_null())  {   this->bio.setEthnicity(QString::fromStdString(i["ethnicity"].as<std::string>())); }
        if (!i["height"].is_null())     {   this->bio.setHeight(Height(i["height"].as<int>()));   /* convert from cm */    }
        if (!i["weight"].is_null())     {   this->bio.setWeight(i["weight"].as<int>());                                    }
        if (!i["measurements"].is_null()){  this->bio.setMeasurements(QString::fromStdString(i["measurements"].as<std::string>()));  }
        if (!i["hair"].is_null())       {   this->bio.setHairColor(QString::fromStdString(i["hair"].as<std::string>()));       }
        if (!i["eyes"].is_null())       {   this->bio.setEyeColor(QString::fromStdString(i["eyes"].as<std::string>()));       }
        if (!i["tattoos"].is_null())    {   this->bio.setTattoos(QString::fromStdString(i["tattoos"].as<std::string>()));    }
        if (!i["piercings"].is_null())  {   this->bio.setPiercings(QString::fromStdString(i["piercings"].as<std::string>()));  }
        if (!name.isEmpty()){
            this->headshot = getProfilePhoto(this->name);
        }
    } catch(std::exception &e){
        qWarning("Error Caught while making Actor from record: %s", e.what());
    }
}

Actor::~Actor(){

}

void Actor::setup(){
    /*
    this->displayItemCreated = false;
    this->itemName = QSharedPointer<QStandardItem>(new QStandardItem());
    this->itemAge = QSharedPointer<QStandardItem>(new QStandardItem());
    this->itemPhoto = QSharedPointer<QStandardItem>(new QStandardItem());

    this->itemName->setText(name);
    QString photo = headshotDownloaded(this->name);
    if (!photo.isEmpty()){
        QFileInfo info(photo);
        if (info.exists()){
            this->headshot = FilePath(info.absoluteFilePath(), info.baseName(), info.suffix());
            this->itemPhoto->setData(QVariant(QPixmap(info.absoluteFilePath()).scaledToHeight(20)));
        }
    }
    */
}
void Actor::setHeadshot(QString s){
    this->headshot = FilePath(s);
}

void Actor::setHeadshot(FilePath f){
    this->headshot = f;
}

Actor Actor::operator =(Actor &obj){
    this->bio = obj.bio;
    this->name = obj.name;
    this->dataUsage = obj.dataUsage;
    this->headshot = obj.headshot;
    return *this;
}

bool Actor::operator < (Actor &other) const {  return (this->name < other.getName());  }
bool Actor::operator > (Actor &other) const {  return (this->name > other.getName());  }
bool Actor::operator ==(Actor &other) const {  return (this->name == other.getName()); }

bool Actor::hasBio(){
    return (this->bio.size() > 2);
}

QSharedPointer<QStandardItem> Actor::getNameItem(){
    return this->itemName;
}

QList<QStandardItem *> Actor::buildQStandardItem(){
    this->row.clear();
    qDebug("Creating Display item for %s", qPrintable(name));
    this->itemName = QSharedPointer<QStandardItem>(new QStandardItem());
//    this->itemAge = QSharedPointer<QStandardItem>(new QStandardItem());
    this->itemPhoto = QSharedPointer<QStandardItem>(new QStandardItem());
    this->itemHair  = ItemPtr(new QStandardItem());
    this->itemEthnicity = ItemPtr(new QStandardItem());

    this->itemName->setText(name);
    if (headshotDownloaded(name)){
        QString photoPath = getProfilePhoto(name);
        qDebug("Profile Photo for %s: %s", qPrintable(name), qPrintable(photoPath));
        if (QFileInfo(photoPath).exists()){
            this->itemPhoto->setData(QVariant(QPixmap(photoPath).scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT)), Qt::DecorationRole);
        } else {
            this->itemPhoto->setData(QVariant(QPixmap(":/Icons/blank_profile_photo.png").scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT)), Qt::DecorationRole);
        }
    }
    QString hair, ethnicity;
    hair = bio.getHairColor();
    ethnicity = bio.getEthnicity();
    this->itemHair->setText(hair);
    this->itemEthnicity->setText(ethnicity);
    this->displayItemCreated = true;
    row << itemPhoto.data() << itemName.data() << itemHair.data() << itemEthnicity.data();
    return row;
}

void Actor::updateQStandardItem(){
    qDebug("Updating Display Item for %s", qPrintable(name));
    if (headshotDownloaded(name)){
        QString photoPath = getProfilePhoto(name);
        qDebug("Profile Photo for %s: %s", qPrintable(name), qPrintable(photoPath));
        if (QFileInfo(photoPath).exists()){
            this->itemPhoto->setData(QVariant(QPixmap(photoPath).scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT)), Qt::DecorationRole);
        } else {
            this->itemPhoto->setData(QVariant(QPixmap(":/Icons/blank_profile_photo.png").scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT)), Qt::DecorationRole);
        }
    }
    if (itemHair->text().isEmpty()){
        QString hair = bio.getHairColor();
        itemHair->setText(hair);
    }
    if (itemEthnicity->text().isEmpty()){
        QString ethnicity = bio.getEthnicity();
        itemEthnicity->setText(ethnicity);
    }
    qDebug("%s's Display Item Updated", qPrintable(name));
}

bool Actor::inDatabase(){
    bool found = false;
    sqlConnection *sql = new sqlConnection(QString("SELECT FROM actors WHERE name = %1").arg(name));
    if (sql->execute()){
        found = sql->foundMatch();
    }
    delete sql;
    return found;
}

Query Actor::toQuery() const{
    Query q;
    q.setTable("actors");
    q.add("NAME",           name);
    q.add("ALIASES",        bio.getAliases());
    if (!bio.getBirthday().isNull() && bio.birthdate.isValid()){
        q.add("BIRTHDAY",       bio.getBirthday());
    }
    q.add("CITY",           bio.getCity());
    q.add("COUNTRY",        bio.getNationality());
    q.add("ETHNICITY",      bio.getEthnicity());
    if (bio.getHeight().nonZero()){
        q.add("HEIGHT",         bio.getHeight().toString());
    }
    q.add("WEIGHT",         bio.getWeight());
    q.add("MEASUREMENTS",   bio.getMeasurements());
    q.add("HAIR",           bio.getHairColor());
    q.add("EYES",           bio.getEyeColor());
    q.add("TATTOOS",        bio.getTattoos());
    q.add("PIERCINGS",      bio.getPiercings());
    q.addCriteria("ID", QString::number(this->ID));
    return q;
}

int Actor::entrySize(){
    int size = bio.size();
    size += (name.isEmpty() ? 0 : 1);
    size += (headshot.isEmpty() ? 0 : 1);
    return size;

}
