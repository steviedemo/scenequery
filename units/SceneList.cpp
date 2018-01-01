#include "SceneList.h"
#include "Scene.h"
#include "Actor.h"
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

int SceneList::countScenesWithActor(ActorPtr a) const{
    return countScenesWithActor(a->getName());
}

int SceneList::countScenesWithActor(QString name) const{
    int count = 0;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if (s->getActors().contains(name)){
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

SceneList SceneList::longerThan(double length) const{
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
SceneList SceneList::shorterThan(double length) const{
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
