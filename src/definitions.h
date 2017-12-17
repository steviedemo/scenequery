#ifndef DEFINITIONS_H
#define DEFINITIONS_H
using List<class T>         = QVector<QSharedPointer<class T>>;
using ListPointer<class T>  = QSharedPointer<QVector<QSharedPointer<class T>>>;
using ScenePtr              = QSharedPointer<class Scene>;
using ActorPtr              = QSharedPointer<class Actor>;
namespace Database{
    enum Operation  { FETCH, UPDATE_TABLE, CLEAN_TABLE, NONE };
    enum Table      { SCENE, ACTOR, THUMBNAIL, HEADSHOT, FILMOGRAPHY };
    enum queryType  { INSERT, UPDATE, REQUEST };
}

#endif // DEFINITIONS_H
