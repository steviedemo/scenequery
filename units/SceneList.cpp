#include "SceneList.h"
#include "Scene.h"
#include "Actor.h"

SceneList::SceneList(const SceneList &s):
    QList<QSharedPointer<Scene>>(){
    qDebug("SceneList copy constructor called");
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        this->push_back(it.next());
    }
}

SceneList SceneList::operator =(const SceneList &s){
    qDebug("SceneList = operator called");
    this->clear();
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        this->push_back(it.next());
    }
    return *this;
}


SceneList SceneList::withCompany(QString c){
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

SceneList SceneList::withActor(QString name){
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

SceneList SceneList::withActor(ActorPtr a){
    return withActor(a->getName());
}

SceneList SceneList::withRating(Rating r){
    QString str = r.toString();
    SceneList list;
    QListIterator<ScenePtr> it(*this);
    while(it.hasNext()){
        ScenePtr s = it.next();
        if (s->getRating().toString() == str){
            list << s;
        }
    }
    return list;
}

SceneList SceneList::withTitle(QString title){
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

SceneList SceneList::inSeries(QString series){
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

SceneList SceneList::longerThan(double length){
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
SceneList SceneList::shorterThan(double length){
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

SceneList SceneList::minResolution(int size){
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

SceneList SceneList::maxResolution(int size){
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
