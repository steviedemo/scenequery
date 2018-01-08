#include "Actor.h"
#include "Scene.h"
#include "SceneList.h"
#include "Query.h"
#include "curlTool.h"
#include "filenames.h"
#include "sql.h"
#include <QRegularExpression>
#include <QStringList>
#include <QPixmap>
#include "curlTool.h"
Actor::Actor(QString name):
    Entry(), name(name), bio(name){
    this->dataUsage = 0.0;
    this->photoPath = getProfilePhoto(name);
    setup();
}
Actor::Actor(const Actor &a):
    Entry(), bio(a.getName()){
    this->bio = a.bio;
    this->name = a.name;
    this->dataUsage = a.dataUsage;
    this->photoPath = a.photoPath;
    setup();
}

Actor::Actor(QString actorName, Biography bio, QString headshot):
    Entry(), name(actorName), bio(bio), dataUsage(0.0), photoPath(headshot){
    setup();
}


Actor::Actor(pqxx::result::const_iterator &i):Entry(){
    this->fromRecord(i);
}

void Actor::fromRecord(pqxx::result::const_iterator i){
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
            this->photoPath = getProfilePhoto(name);
        }
    } catch(std::exception &e){
        qWarning("Error Caught while making Actor from record: %s", e.what());
    }
    setup();
}

Actor::~Actor(){

}

void Actor::setup(){
    this->sceneCount = 0;
    this->sceneList = {};
}

void Actor::addScene(ScenePtr s){
    this->sceneList.push_back(s);
    this->sceneCount++;
    if (this->itemSceneCount.isNull()){
        this->itemSceneCount = ItemPtr(new QStandardItem());
    }
    this->itemSceneCount->setData(QVariant(sceneCount), Qt::DecorationRole);
}
void Actor::addScene(void){
    this->sceneCount++;
    if (this->itemSceneCount.isNull()){
        this->itemSceneCount = ItemPtr(new QStandardItem());
    }
    this->itemSceneCount->setData(QVariant(sceneCount), Qt::DecorationRole);
}

void Actor::setScenes(SceneList list){
    this->sceneList = list;
    if (this->itemSceneCount.isNull()){
        this->itemSceneCount = ItemPtr(new QStandardItem());
    }
    this->itemSceneCount->setData(QVariant(sceneCount), Qt::DecorationRole);
}

int Actor::size(){
    int size = bio.size();
    if (photoPath != DEFAULT_PROFILE_PHOTO) {   size++;    }
    if (!name.isEmpty()){   size++; }
    return size;
}

Actor Actor::operator =(Actor &obj){
    this->bio = obj.bio;
    this->name = obj.name;
    this->dataUsage = obj.dataUsage;
    this->photoPath = obj.photoPath;
    return *this;
}

bool Actor::operator < (Actor &other) const {  return (this->name < other.getName());  }
bool Actor::operator > (Actor &other) const {  return (this->name > other.getName());  }
bool Actor::operator ==(Actor &other) const {  return (this->name == other.getName()); }

bool Actor::hasBio(){
    return (this->bio.size() > 2);
}

bool Actor::isEmpty()               {   return name.isEmpty();          }
bool Actor::usingDefaultPhoto()     {   return (this->photoPath == DEFAULT_PROFILE_PHOTO);    }


void Actor::setScaledProfilePhoto(QVariant profile){
    this->profilePhoto = profile;
}

void Actor::buildScaledProfilePhoto()   {
    qDebug("Building Scaled Profile Photo for %s", qPrintable(name));
    if (photoPath.isEmpty()){
        photoPath = getProfilePhoto(name);
    }
    this->profilePhoto = QVariant(QPixmap(photoPath).scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT));
}

void Actor::setDefaultHeadshot(){
    this->photoPath = DEFAULT_PROFILE_PHOTO;
    this->profilePhoto = QVariant(QPixmap(DEFAULT_PROFILE_PHOTO).scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT));
    this->itemPhoto->setData(QVariant(profilePhoto), Qt::DecorationRole);
}
void Actor::setHeadshot(QString fileLocation)  {
    this->photoPath = fileLocation;
}

QSharedPointer<QStandardItem> Actor::getNameItem(){
    return this->itemName;
}
QList<QStandardItem *> Actor::getQStandardItem(){
    return row;
}

QList<QStandardItem *> Actor::buildQStandardItem(){
    //qDebug("Creating Display item for %s", qPrintable(name));
    this->itemName      = ItemPtr(new QStandardItem(name));
    this->itemHair      = ItemPtr(new QStandardItem(bio.getHairColor()));
    this->itemEthnicity = ItemPtr(new QStandardItem(bio.getEthnicity()));
    this->itemSceneCount= ItemPtr(new QStandardItem(QString::number(sceneCount)));
    this->itemPhoto     = ItemPtr(new QStandardItem());

    this->itemSceneCount->setData(QVariant(sceneCount), Qt::DecorationRole);
    this->itemSceneCount->setTextAlignment(Qt::AlignLeft);
    QString photo = getProfilePhoto(name);
    this->itemPhoto->setData(QVariant(QPixmap(photo).scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT)), Qt::DecorationRole);
    /*
    if (!this->profilePhoto.isNull() && this->profilePhoto.isValid()){
        this->itemPhoto->setData(profilePhoto, Qt::DecorationRole);
    } else {
        this->itemPhoto->setData(QVariant(QPixmap(DEFAULT_PROFILE_PHOTO).scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT)), Qt::DecorationRole);
    }*/
    row << itemPhoto.data() << itemName.data() << itemHair.data() << itemEthnicity.data() << itemSceneCount.data();
    this->displayItemCreated = true;
    //qDebug("%s's Display Item Created", qPrintable(name));
    return row;
}

void Actor::updateQStandardItem(){
    qDebug("Updating Display Item for %s", qPrintable(name));
    this->photoPath = getProfilePhoto(name);
    this->itemSceneCount->setText(QString::number(sceneList.size()));
    QString photo = getProfilePhoto(name);
    this->itemPhoto->setData(QVariant(QPixmap(photo).scaledToHeight(ACTOR_LIST_PHOTO_HEIGHT)), Qt::DecorationRole);
    if (itemHair->text().isEmpty()){
        QString hair = bio.getHairColor();
        itemHair->setText(hair);
    }
    if (itemEthnicity->text().isEmpty()){
        QString ethnicity = bio.getEthnicity();
        itemEthnicity->setText(ethnicity);
    }
    this->itemSceneCount->setData(QVariant(sceneCount), Qt::DecorationRole);
    qDebug("%s's Display Item Updated", qPrintable(name));
}
void Actor::setBio(const Biography &other){
    qDebug("Copying new Biography into %s's profile", qPrintable(name));
    this->bio.copy(other);
    qDebug("%s's Bio is now of Size %d", qPrintable(name), this->bio.size());
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

int Actor::entrySize(){
    int size = bio.size();
    size += (name.isEmpty() ? 0 : 1);
    if (!usingDefaultPhoto()){
        size++;
    }
    return size;

}
