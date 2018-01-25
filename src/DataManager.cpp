#include "DataManager.h"

DataManager::DataManager(QObject *parent):
    QObject(parent){
    this->sceneUpdateList = {};
    this->actorUpdateList = {};
    this->actorMap = QHash<QString, ActorPtr>();
    this->sceneMap = QHash<int, ScenePtr>();
}

DataManager::~DataManager(){}

bool DataManager::contains(const int ID) const{
    mx.lock();
    bool hasID = sceneMap.contains(ID);
    mx.unlock();
    return hasID;
}
bool DataManager::contains(const QString &name) const{
    mx.lock();
    bool hasName = actorMap.contains(name);
    mx.unlock();
    return hasName;
}

bool DataManager::add(const ActorPtr a){
    bool dataValid = false;
    if (!a.isNull()){
        const QString name = a->getName();
        if (!name.isEmpty()){
            dataValid = true;
            mx.lock();
            if (!actorMap.contains(name)){
                actorMap.insert(name, a);
            }
            mx.unlock();
        }
    } else {
        qWarning("Not inserting empty actor into map.");
    }
    return dataValid;
}
bool DataManager::add(const ScenePtr s){
    bool dataValid = true;
    if(!s.isNull()){
        const int id = s->getID();
        if (id > 0){
            dataValid = true;
            mx.lock();
            if (!sceneMap.contains(id)){
                sceneMap.insert(id, s);
            }
            mx.unlock();
        }
    } else {
        qWarning("Not inserting empty Scene into map");
    }
    return dataValid;
}

bool DataManager::update(const ActorPtr a, bool saveToDB){
    bool dataValid = false;
    if (!a.isNull()){
        const QString name = a->getName();
        if (!name.isEmpty()){
            dataValid = true;
            mx.lock();
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
            mx.unlock();
            if (saveToDB){
                emit save(a);
            }
        }
    } else {
        qWarning("Not inserting empty actor into map.");
    }
    return dataValid;
}

bool DataManager::update(const ScenePtr s, bool saveToDB){
    bool dataValid = true;
    if (!s.isNull()){
        const int id = s->getID();
        if (id > 0){
            mx.lock();
            if (!sceneMap.contains(id)){
                sceneMap.insert(id, s);
                if (saveToDB){
                    emit save(s);
                }
            }
            mx.unlock();
            dataValid = true;
        }
    }
    return dataValid;
}
void DataManager::update(const ActorList list, bool saveToDB){
    foreach(ActorPtr a, list){  update(a, saveToDB); }
}
void DataManager::add(const ActorList list){
    foreach(ActorPtr a, list){ add(a);  }
}
void DataManager::update(const SceneList list, bool saveToDB){
    foreach(ScenePtr s, list){  update(s, saveToDB); }
}
void DataManager::add(const SceneList list){    foreach(ScenePtr s, list){  add(s); }   }

ActorPtr DataManager::getActor(const QString name) const{
    ActorPtr a = ActorPtr(0);
    mx.lock();
    if (actorMap.contains(name)){
        a = actorMap.value(name);
    } else {
      qWarning("%s is not in the actor map.", qPrintable(name));
    }
    mx.unlock();
    return a;
}
ScenePtr DataManager::getScene(const int id) const{
    ScenePtr s = ScenePtr(0);
    mx.lock();
    if (sceneMap.contains(id)){
        s = sceneMap.value(id);
    } else {
        qWarning("Scene with ID '%d' is not in the scene map", id);
        s = ScenePtr(0);
    }
    mx.unlock();
    return s;
}
void DataManager::remove(const QString &name){
    if (!name.isEmpty()){
        mx.lock();
        if (actorMap.contains(name)){
            qDebug("Removing %s from the actor Map", qPrintable(name));
            actorMap.remove(name);
        } else {
            qWarning("Actor '%s' is not in the actor map", qPrintable(name));
        }
        mx.unlock();
    } else {
        qWarning("Cannot remove actor with Empty Name from Actor Map");
    }
}
void DataManager::remove(const int id){
    QMutexLocker ml(&mx);
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
    mx.lock();
    if (actorMap.contains(name)){
        birthdate = actorMap.value(name)->getBirthday();
    }
    mx.unlock();
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
    QMutexLocker ml(&mx);
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
    QMutexLocker ml(&mx);
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
        mx.lock();
        actorMap[it.key()]->updateQStandardItem();
        mx.unlock();
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
        mx.lock();
        sceneMap[it.key()]->updateQStandardItem();
        mx.unlock();
        emit progressUpdate(++index);
    }
    emit progressEnd(QString("%1 Actor Display Items updated").arg(sceneMap.size()));
    qDebug("%d actor display items updated", sceneMap.size());
}

void DataManager::updateBios(){
    mx.lock();
    this->actorUpdateList.clear();
    QHashIterator<QString, ActorPtr> it(actorMap);
    mx.unlock();
    while(it.hasNext()){
        it.next();
        if (it.value()->size() < MINIMUM_BIO_SIZE){
            mx.lock();
            actorUpdateList << it.value();
            mx.unlock();
        }
    }
    emit updateBiosFromWeb(actorUpdateList);
}
