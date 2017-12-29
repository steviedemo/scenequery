#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <QSharedPointer>
#include <QVector>
//template <typename T> using List = QVector<QSharedPointer<T>>;
//template <typename T> using ListPointer = QSharedPointer<QVector<QSharedPointer<T>>>;
//#define tr();    qDebug("%s::%s::%d", __FILE__, __FUNCTION__, __LINE__);

#define IMAGE_WIDTH 250
#define IMAGE_HEIGHT 300
typedef QSharedPointer<class Scene>         ScenePtr;
typedef QSharedPointer<class Actor>         ActorPtr;
typedef QSharedPointer<class QStandardItem> ItemPtr;
//using SceneList             = QVector<QSharedPointer<class Scene>>;
using ActorList             = QVector<QSharedPointer<class Actor>>;

#endif // DEFINITIONS_H
