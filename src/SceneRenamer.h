#ifndef SCENERENAMER_H
#define SCENERENAMER_H
#include <QString>
#include <QDate>
#include <QStringList>
#include "Rating.h"
#include "Scene.h"
#include "definitions.h"
class SceneRenamer
{
private:
    ScenePtr current;
    QVector<QString> actors, tags;
    QString folder, extension, newFilename, oldFilename, title, company, series, releaseString, rating, tagString;
    QString mainActor, featuredActors;
    int actorCount;
    int height, sceneNumber;
    bool sceneOk;
public:
    SceneRenamer(ScenePtr s);
    SceneRenamer(Scene *s);

    QString getNewFilename();
    QString displayInfo();
protected:
    QString formatActor();
    QString formatCompany();
    QString formatTitle();
    QString formatParentheses();
    QString makeTagString(QVector<QString>) const;
    QString makeBracketString();
    QString makeFeaturedString(QVector<QString>) const;
private:
    void scan(ScenePtr);
};

#endif // SCENERENAMER_H
