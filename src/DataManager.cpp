#include "DataManager.h"

DataManager::DataManager(QObject *parent):
    QObject(parent){
    this->sceneUpdateList = {};
    this->actorUpdateList = {};
    this->actorMap = QMap<QString, ActorPtr>();
    this->sceneMap = QMap<int, ScenePtr>();
}

DataManager::~DataManager(){}

bool DataManager::add(const ActorPtr a, bool saveToDB){
    bool dataValid = false;
    if (!a.isNull()){
        const QString name = a->getName();
        if (!name.isEmpty()){
            dataValid = true;
            if (!actorMap.contains(name)){
                actorMap.insert(name, a);
            } else {
                Biography bio = a->getBio();
                if (bio.size() > actorMap.value(name)->getBio().size()){
                    actorMap[name]->setBio(bio);
                    qDebug("Updating %s's Display Item...", qPrintable(name));
                    actorMap[name]->updateQStandardItem();
                    qDebug("Display Item Updated");
                }
            }
            if (saveToDB){
                emit save(a);
            }
        }
    } else {
        qWarning("Not inserting empty actor into map.");
    }
    return dataValid;
}

bool DataManager::add(const ScenePtr s, bool saveToDB){
    bool dataValid = true;
    if (!s.isNull()){
        const int id = s->getID();
        if (id > 0){
            if (!sceneMap.contains(id)){
                sceneMap.insert(id, s);
                if (saveToDB){
                    emit save(s);
                }
            }
            dataValid = true;
        }
    }
    return dataValid;
}
void DataManager::add(const ActorList list, bool saveToDB){
    foreach(ActorPtr a, list){  add(a); }
}
void DataManager::add(const SceneList list){
    foreach(ScenePtr s, list){  add(s); }
}

ActorPtr DataManager::getActor(const QString name) const{
    if (actorMap.contains(name)){
        return actorMap.value(name);
    } else {
      qWarning("%s is not in the actor map.", qPrintable(name));
      return ActorPtr(0);
    }
}
ScenePtr DataManager::getScene(const int id) const{
    if (sceneMap.contains(id)){
        return sceneMap.value(id);
    } else {
        qWarning("Scene with ID '%d' is not in the scene map", id);
        return ScenePtr(0);
    }
}
void DataManager::remove(const QString &name){
    if (!name.isEmpty()){
        if (actorMap.contains(name)){
            qDebug("Removing %s from the actor Map", qPrintable(name));
            actorMap.remove(name);
        } else {
            qWarning("Actor '%s' is not in the actor map", qPrintable(name));
        }
    } else {
        qWarning("Cannot remove actor with Empty Name from Actor Map");
    }
}
void DataManager::remove(const int id){
    if (sceneMap.contains(id)){
        qDebug("Removing Scene with ID '%d' from Scene Map", id);
        sceneMap.remove(id);
    } else {
        qWarning("Scene with ID '%d' is not in the Scene Map", id);
    }
}
void DataManager::remove(const ActorPtr a){
    if (!a.isNull()){
        const QString name = a->getName();
        remove(name);
    } else {
        qWarning("Cannot remove Null Object from actor map");
    }
}
void DataManager::remove(const ScenePtr s){
    if (!s.isNull()){
        const int id = s->getID();
        remove(id);
    } else {
        qWarning("Cannot Remove Null Object from Scene Map");
    }
}

QDate DataManager::getBirthday(const QString &name) const{
    QDate birthdate;
    if (actorMap.contains(name)){
        birthdate = actorMap.value(name)->getBirthday();
    }
    return birthdate;
}

int DataManager::getAge(const QString &name, const QDate &date) const{
    int age = -1;
    if (!date.isNull() && date.isValid()){
        QDate birthday = getBirthday(name);
        if (!birthday.isNull() && birthday.isValid()){
            age = (birthday.daysTo(date)/365);
        }
    }
    return age;
}

void DataManager::saveAllActors(){
    this->actorUpdateList.clear();
    QHashIterator<QString, ActorPtr> it(actorMap);
    while(it.hasNext()){
        it.next();
        actorUpdateList << it.value();
    }
    if (actorUpdateList.size() > 0){
        qDebug("Saving %d Actors to the Database", actorUpdateList.size());
        emit save(actorUpdateList);
    }
}

void DataManager::saveAllScenes(){
    this->sceneUpdateList.clear();
    QHashIterator<int, ScenePtr> it(sceneMap);
    while(it.hasNext()){
        it.next();
        sceneUpdateList << it.value();
    }
    if (sceneUpdateList.size() > 0){
        qDebug("Saving %d Scenes to the Database", sceneUpdateList.size());
        emit save(sceneUpdateList);
    }
}

void DataManager::updateActorDisplayItems(){
    int index = 0;
    emit progressBegin(QString("Updating %1 Actor Display Items").arg(actorMap.size()), actorMap.size());
    QHashIterator<QString, ActorPtr> it(actorMap);
    while(it.hasNext()){
        it.next();
        actorMap[it.key()]->updateQStandardItem();
        emit progressUpdate(++index);
    }
    emit progressEnd(QString("%1 Actor Display Items updated").arg(actorMap.size()));
    qDebug("%d actor display items updated", actorMap.size());
}

void DataManager::updateSceneDisplayItems(){
    int index = 0;
    emit progressBegin(QString("Updating %1 Scene Display Items").arg(sceneMap.size()), sceneMap.size());
    QHashIterator<int, ScenePtr> it(sceneMap);
    while(it.hasNext()){
        it.next();
        sceneMap[it.key()]->updateQStandardItem();
        emit progressUpdate(++index);
    }
    emit progressEnd(QString("%1 Actor Display Items updated").arg(sceneMap.size()));
    qDebug("%d actor display items updated", sceneMap.size());
}

void DataManager::updateBios(){
    this->actorUpdateList.clear();
    QHashIterator<QString, ActorPtr> it(actorMap);
    while(it.hasNext()){
        it.next();
        if (it.value()->size() < MINIMUM_BIO_SIZE){
            actorUpdateList << it.value();
        }
    }
    emit updateBiosFromWeb(actorUpdateList);
}
