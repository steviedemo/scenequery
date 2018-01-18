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
    QString folder, extension, newFilename, oldFilename, title, company, series, releaseString, rating, tagString;
    QStringList actors, tags;
    int actorCount;
    int height, sceneNumber;
    ScenePtr current;
    bool sceneOk;
public:
    SceneRenamer(Scene *s);
    SceneRenamer(ScenePtr s);

    QString getNewFilename();
    QString displayInfo();
protected:
    QString formatActor();
    QString formatCompany();
    QString formatTitle();
    QString formatParentheses();
    QString makeTagString(QStringList) const;
    QString makeBracketString();
    QString makeFeaturedString(QStringList) const;
private:
    void scan(ScenePtr);
};

#endif // SCENERENAMER_H
