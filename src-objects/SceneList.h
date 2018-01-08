#ifndef SCENELIST_H
#define SCENELIST_H
#include "Scene.h"
#include <QSharedPointer>
#include "definitions.h"
class SceneList : public QList<QSharedPointer<Scene>>
{
public:
    SceneList() : QList<QSharedPointer<Scene>>(){}
    //bool contains(const QSharedPointer<Scene> &t) const;
    SceneList withCompany(QString) const;
    SceneList withActor(ActorPtr) const;
    SceneList withActor(QString name) const;
    SceneList withRating(class Rating r) const;
    SceneList longerThan(QTime length) const;
    SceneList shorterThan(QTime length) const;
    SceneList minResolution(int h) const;
    SceneList maxResolution(int h) const;
    SceneList inSeries(QString s) const;
    SceneList withTitle(QString s) const;
    int countScenesWithActor(QString s) const;
    int countScenesWithActor(ActorPtr a) const;

};

#endif // SCENELIST_H
