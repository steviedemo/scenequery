#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <QSharedPointer>
#include <QVector>
#include <QMapIterator>

#define IMAGE_WIDTH 250
#define IMAGE_HEIGHT 300
#define DEFAULT_PROFILE_PHOTO ":/Icons/blank_profile_photo_female.png"
#define ACTOR_LIST_PHOTO_HEIGHT 40
enum RunMode {  Debug, Release };
//#define tr();    qDebug("%s::%s::%d", __FILE__, __FUNCTION__, __LINE__);
typedef QSharedPointer<class Scene>         ScenePtr;
typedef QSharedPointer<class Actor>         ActorPtr;
typedef QSharedPointer<class QStandardItem> ItemPtr;
using ItemList           = QList<QSharedPointer<class QStandardItem>>;
//using SceneList             = QVector<QSharedPointer<class Scene>>;
using ActorList             = QVector<QSharedPointer<class Actor>>;
using ActorMap              = QMap<QString, ActorPtr>;
using ActorIterator         = QMapIterator<QString, ActorPtr>;
#endif // DEFINITIONS_H
