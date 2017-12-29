#ifndef SCENELIST_H
#define SCENELIST_H
#include "Scene.h"
#include <QSharedPointer>
#include "definitions.h"
class SceneList : public QList<QSharedPointer<Scene>>
{
public:
    SceneList() : QList<QSharedPointer<Scene>>(){}
    SceneList(const SceneList &s);
    SceneList operator=(const SceneList &s);
    friend SceneList operator+(const SceneList &s1, const SceneList &s2);
    SceneList withCompany(QString);
    SceneList withActor(ActorPtr);
    SceneList withActor(QString name);
    SceneList withRating(class Rating r);
    SceneList longerThan(double length);
    SceneList shorterThan(double length);
    SceneList minResolution(int h);
    SceneList maxResolution(int h);
    SceneList inSeries(QString s);
    SceneList withTitle(QString s);
};

#endif // SCENELIST_H
