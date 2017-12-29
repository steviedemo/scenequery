#include "filenames.h"
#include "config.h"
#include <QFileInfo>
#include "Scene.h"
#include "Actor.h"
#include <QRegularExpression>
#include <QPixmap>
#define DEFAULT_PROFILE_PHOTO ":/Icons/blank_profile_photo.png"

/** \brief  Format an actor's name into a filename that doesn't contain characters that need escaping.
 *  \param  QString actorName:  Name of Actor
 *  \return QString fullPath:   Absolute path to the file.
 */
QString getHeadshotName(QString actorName){
    QString name_copy = QString("%1").arg(actorName);
    QString filename = name_copy.trimmed().replace(QRegularExpression("[\\s\\.']"), "_").toLower();
    QString fullPath = QString("%1/%2.jpg").arg(findHeadshotLocation()).arg(filename);
    return fullPath;
}


/** \brief  Check the filesystem for the existence of the headshot for the actor with the name provided.
 *  \param  QString actorName:  name of actor.
 *  \return QString filename:   Path to file if it exists, Empty string if it doesn't exist.
 */
bool headshotDownloaded(QString actorName){
    bool downloaded = false;
    QString filepath = getHeadshotName(actorName);
    QFile f(filepath);
    qDebug("Searching for %s's headshot (filename: %s)...", qPrintable(actorName), qPrintable(filepath));
    if (f.exists()){
        downloaded = (QFileInfo(f).size() > 200);
    }
    return downloaded;
}

QString getProfilePhoto(QString actorName){
    QString location = DEFAULT_PROFILE_PHOTO;
    if (headshotDownloaded(actorName)){
        location = getHeadshotName(actorName);
    }
    return location;
}

/** \brief Format a scene's data to get a unique, usable filename for each thumbnail */
QString getThumbnailName(ScenePtr s, int number){
    QString title = QString("%1_%2_%3_%4").arg(s->getActor()).arg(s->getTitle()).arg(s->getID()).arg(number);
    QString filename = title.trimmed().replace(QRegularExpression("[\\s\\.'\\]\\[\\)\\(]"), "_");
    QString fullPath = QString("%1/%2.png").arg(findThumbnailLocation()).arg(filename);
    return fullPath;
}

bool thumbnailExists(ScenePtr s){
    return QFileInfo(getThumbnailName(s, 1)).exists();
}
