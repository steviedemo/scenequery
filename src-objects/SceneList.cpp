#include "SceneList.h"
#include "Scene.h"
#include "Actor.h"
#include <QHash>
SceneList scenesWithActor(const QString name, const QHash<int, ScenePtr> &hash){
    SceneList list = {};
    QHashIterator<int,ScenePtr> it(hash);
    while(it.hasNext()){
        it.next();
        if (it.value()->hasActor(name)){
            list.append(it.value());
        }
    }
    return list;
}

int countWithActor(const QString name, const QHash<int, ScenePtr> &hash){
    int count = 0;
    QHashIterator<int,ScenePtr> it(hash);
    while(it.hasNext()){
        it.next();
        if (it.value()->hasActor(name)){
            ++count;
        }
    }
    return count;
}

SceneList fromHashMap(const QHash<int, ScenePtr>&hash){
    SceneList list = {};
    QHashIterator<int,ScenePtr> it(hash);
    while(it.hasNext()){
        it.next();
        list.append(it.value());
    }
    return list;
}

/*
bool SceneList::contains(const QSharedPointer<Scene> &t) const{
    bool found = false;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext() && !found){
        ScenePtr s = it.next();
        if (s->equals(t)){
            found = true;
        }
    }
    return found;
}
*/
SceneList SceneList::withCompany(QString c) const{
    SceneList newList;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if (s->getCompany() == c){
            newList << s;
        }
    }
    return newList;
}

ScenePtr SceneList::getScene(const QString &filepath){
    bool found = false;
    ScenePtr scene = ScenePtr(0);
    if (valid(filepath)){
        QListIterator<ScenePtr> it(*this);
        while(it.hasNext() && !found){
            ScenePtr curr = it.next();
            if (!curr.isNull()){
                if (curr->equals(filepath)){
                    found = true;
                    scene = curr;
                    break;
                }
            }
        }
    }
    return scene;
}

ScenePtr SceneList::getScene(const QPair<QString, QString> &filepath){
    ScenePtr requestedScene = ScenePtr(0);
    if (valid(filepath.first) && valid(filepath.second)){
        QListIterator<ScenePtr> it(*this);
        bool found = false;
        while(it.hasNext() && !found){
            ScenePtr s = it.next();
            if (!s.isNull() && s->hasValidFile()){
                if (s->getFilename() == filepath.second && s->getFolder() == filepath.first){
                    requestedScene = s;
                    found = true;
                }
            }
        }
    }
    return requestedScene;
}

int SceneList::countScenesWithActor(ActorPtr a) const{
    return countScenesWithActor(a->getName());
}

int SceneList::countScenesWithActor(QString name) const{
    int count = 0;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if (s->hasActor(name)){
            ++count;
        }
    }
    return count;
}

SceneList SceneList::withActor(QString name) const{
    SceneList newList;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if (s->getActors().contains(name)){
            newList << s;
        }
    }
    return newList;
}

SceneList SceneList::withActor(ActorPtr a) const{
    return withActor(a->getName());
}

SceneList SceneList::withRating(Rating r) const{
    int stars = r.stars();
    SceneList list;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if (s->getRating().stars() == stars){
            list << s;
        }
    }
    return list;
}

SceneList SceneList::withTitle(QString title) const{
    SceneList list;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if(s->getTitle() == title){
            list << s;
        }
    }
    return list;
}

SceneList SceneList::inSeries(QString series) const{
    SceneList list;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if(s->getSeries() == series){
            list << s;
        }
    }
    return list;
}

SceneList SceneList::longerThan(QTime length) const{
    SceneList list;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if(s->getLength() > length){
            list << s;
        }
    }
    return list;
}

SceneList SceneList::shorterThan(QTime length) const{
    SceneList list;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if(s->getLength() < length){
            list << s;
        }
    }
    return list;
}

SceneList SceneList::minResolution(int size) const{
    SceneList list;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if(s->getHeight() > size){
            list << s;
        }
    }
    return list;
}

SceneList SceneList::maxResolution(int size) const{
    SceneList list;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if(s->getHeight() < size){
            list << s;
        }
    }
    return list;
}
