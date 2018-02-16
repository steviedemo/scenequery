#include "DataManager.h"

DataManager::DataManager(QObject *parent):
    QObject(parent){
    this->sceneUpdateList = {};
    this->actorUpdateList = {};
    this->actorMap = QHash<QString, ActorPtr>();
    this->sceneMap = QMap<int, ScenePtr>();
}

DataManager::~DataManager(){}

bool DataManager::contains(const int ID) const{
    return sceneMap.contains(ID);
}
bool DataManager::contains(const QString &name) const{
    return actorMap.contains(name);
}

void DataManager::reparse(const int id){
    if (sceneMap.contains(id)){
        sceneMap[id]->reparse();
        save(id);
    } else {
        qWarning("Error: Map Doesn't Contain ID %d, Cannot Reparse Item.", id);
    }
}

bool DataManager::save(const int id){
    bool success = false;
    if (sceneMap.contains(id)){
        success = sceneMap.value(id)->save();
        sceneMap.value(id)->updateQStandardItem();
    } else {
        qWarning("Error: Map doesn't Contain ID %d, cannot save.", id);
    }
    return success;
}
bool DataManager::save(const QString name){
    bool success = false;
    if (actorMap.contains(name)){
        success = actorMap.value(name)->save();
        actorMap.value(name)->updateQStandardItem();
    } else {
        qWarning("Error: Map doesn't Contain Name '%s': Cannot Save.", qPrintable(name));
    }
    return success;
}

void DataManager::updateDisplayItem(const int id){
    if (contains(id)){
        ScenePtr s = this->sceneMap.value(id);
        if (s.isNull()){
            qWarning("Error: Scene with the ID %d is Null", id);
        } else {
            s->updateQStandardItem();
        }
    }
}
void DataManager::updateDisplayItem(const QString name){
    if (contains(name)){
        ActorPtr a = this->actorMap.value(name);
        if (a.isNull()){
            qWarning("Error: Vault entry for '%s' returned a null value", qPrintable(name));
        } else {
            a->updateQStandardItem();
        }
    }
}
QList<QStandardItem *> DataManager::buildQStandardItem(const int id){
    QList<QStandardItem *> row;
    if (contains(id)){
        row = sceneMap[id]->buildQStandardItem();
    }
    return row;
}
QList<QStandardItem *> DataManager::buildQStandardItem(const QString name){
    QList<QStandardItem *> row;
    if (contains(name)){
        row = actorMap[name]->buildQStandardItem();
    }
    return row;
}

void DataManager::setMap(ActorMap actors){
    QMutexLocker ml(&mx);
    this->actorMap = actors;
    if (!sceneMap.isEmpty()){
        mapActorsToScenes();
    }
    emit statusUpdate(QString("Added %1 Actors").arg(actors.size()));
}
void DataManager::setMap(SceneMap scenes){
    QMutexLocker ml(&mx);
    this->sceneMap = scenes;
    if (!actorMap.isEmpty()){
        mapActorsToScenes();
    }
    emit statusUpdate(QString("Added %1 Scenes").arg(scenes.size()));
}

void DataManager::mapActorsToScenes(){
    QHashIterator<QString, ActorPtr> it(actorMap);
    while(it.hasNext()){
        it.next();
        this->actorSceneMap.insert(it.key(), QVector<ScenePtr>(0));
    }
    QMapIterator<int, ScenePtr> jt(sceneMap);
    while(jt.hasNext()){
        jt.next();
        foreach(QString s, jt.value()->getActors()){
            actorSceneMap[s].push_back(jt.value());
        }
    }
}

QVector<ScenePtr> DataManager::getActorsScenes(const QString name){
    QVector<ScenePtr> list = {};
    if (actorSceneMap.contains(name)){
        list = actorSceneMap.value(name);
    }
    //    SceneList list = {};
//    if (!name.isEmpty()){
//        qDebug("Gathering Scenes for actor '%s'", qPrintable(name));
//        int index = 0;
//        QMapIterator<int, ScenePtr> it(sceneMap);
//        while(it.hasNext()){
//            it.next();
//            ScenePtr s = it.value();
//            if (!s.isNull()){
//                if (s->hasActor(name)){
//                    list << it.value();
//                }
//            }
//        }
//        qDebug("Returning %d scenes for '%s'", list.size(), qPrintable(name));
//    }
    return list;
}

bool DataManager::add(const ActorPtr a, bool saveToDB){
    bool dataValid = false;
    if (!a.isNull()){
        const QString name = a->getName();
        if (!name.isEmpty()){
            dataValid = true;
            mx.lock();
            if (!actorMap.contains(name)){
                actorMap.insert(name, a);
                if (saveToDB){
                    emit save(a);
                }
            }
            mx.unlock();
        }
    } else {
        qWarning("Not inserting empty actor into map.");
    }
    return dataValid;
}
bool DataManager::add(const ScenePtr s, bool saveToDB){
    bool dataValid = true;
    if(!s.isNull()){
        const int id = s->getID();
        if (id > 0){
            dataValid = true;
            mx.lock();
            if (!sceneMap.contains(id)){
                sceneMap.insert(id, s);
                if (saveToDB){
                    emit save(s);
                }
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
void DataManager::add(const ActorList list, bool saveToDatabase){
    /// Don't save to the database if we're doing the initial load from the database
    foreach(ActorPtr a, list){ add(a, (!actorMap.isEmpty() && saveToDatabase));  }
}
void DataManager::update(const SceneList list, bool saveToDB){
    foreach(ScenePtr s, list){  update(s, saveToDB); }
}
void DataManager::add(const SceneList list, bool saveToDatabase){
    foreach(ScenePtr s, list){  add(s, saveToDatabase && !sceneMap.isEmpty()); }
}

ActorPtr DataManager::getActor(const QString name){
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
ScenePtr DataManager::getScene(const int id) {
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

QDate DataManager::getBirthday(const QString &name) {
    QDate birthdate;
    mx.lock();
    if (actorMap.contains(name)){
        birthdate = actorMap.value(name)->getBirthday();
    }
    mx.unlock();
    return birthdate;
}

int DataManager::getAge(const QString &name, const QDate &date){
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
    QMapIterator<int, ScenePtr> it(sceneMap);
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
    QMapIterator<int, ScenePtr> it(sceneMap);
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
