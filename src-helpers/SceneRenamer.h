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
    QString getNewFilename();
    QString displayInfo();
protected:
    QString formatActor();
    QString formatCompany();
    QString formatTitle();
    QString formatParentheses();
    void doubleCheckNames();
    QString makeTagString(QStringList) const;
    QString makeBracketString();
    QString makeFeaturedString(QStringList) const;
};

#endif // SCENERENAMER_H
