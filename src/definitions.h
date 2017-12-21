#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <QSharedPointer>
#include <QVector>
//template <typename T> using List = QVector<QSharedPointer<T>>;
//template <typename T> using ListPointer = QSharedPointer<QVector<QSharedPointer<T>>>;

typedef QSharedPointer<class Scene> ScenePtr;
typedef QSharedPointer<class Actor> ActorPtr;
using SceneList             = QVector<QSharedPointer<class Scene>>;
using ActorList             = QVector<QSharedPointer<class Actor>>;
namespace Database{
    enum Operation  { OPERATION_FETCH, OPERATION_UPDATE, OPERATION_CLEAN, OPERATION_NONE };
    enum Table      { SCENE, ACTOR, THUMBNAIL, HEADSHOT, FILMOGRAPHY };
}
enum queryType  { SQL_INSERT, SQL_UPDATE, SQL_REQUEST };

#define HOST        "localhost"
#define USERNAME    "derby"
#define PASSWORD    "smashed"
#define SCENE_DB    "scenes"
#define ACTOR_DB    "actors"


#endif // DEFINITIONS_H
