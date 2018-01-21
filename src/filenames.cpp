#include "filenames.h"
#include "config.h"
#include <QFileInfo>
#include "Scene.h"
#include "Actor.h"
#include <QRegularExpression>
#include <QPixmap>
#include <QImageWriter>
#define DEFAULT_PROFILE_PHOTO       ":/Icons/blank_profile_photo_female.png"
#define DEFAULT_PROFILE_PHOTO_THUMB ":/Icons/blank_profile_photo_female_thumb.png"

/** \brief take a string containing a file name or absolute path, and extract the file extension, before returning it.
 *          If an empty string or a string violating the expected format is passed, an empty string is returned.
 */
QString getExtension(const QString &filename){
    QString ext("");
    QRegularExpression rx(".+\\.(.+)");
    QRegularExpressionMatch match = rx.match(filename);
    if (match.hasMatch()){
        ext = match.captured(1);
    }
    return ext;
}

QString toAbsolutePath(const QPair<QString, QString> &file){
    return QString("%1/%2").arg(file.first).arg(file.second);
}

bool isEmpty(const QPair<QString, QString> &file){
    return (file.first.isEmpty() || file.second.isEmpty());
}

/** \brief Split an absolute path into a pair of strings containing the directory path & filename.
 *  \param QString path:    Absolute path of the file
 *  \return QPair<QString,QString>: Tuple containing filepath & filename.
 *          If the param string is empty or breaks the expectations of the format, the 2 strings returned will be empty.
 */
QPair<QString,QString> splitAbsolutePath(const QString path){
    QPair<QString,QString>pair;
    splitAbsolutePath(path, pair.first, pair.second);
    return pair;
}

bool splitAbsolutePath(const QString path, QString &filepath, QString &filename){
    QRegularExpression rx("(\\/.+)\\/(.+)");
    QRegularExpressionMatch match = rx.match(path);
    filepath = "";
    filename = "";
    bool success = false;
    if (match.hasMatch()){
        filepath = match.captured(1);
        filename = match.captured(2);
        success = true;
        //qDebug("Filename: '%s', Filepath: '%s'", qPrintable(pair.second), qPrintable(pair.first));
    } else {
        qWarning("%s encountered Error Parsing filepath & filename out of the string '%s'", __FUNCTION__, qPrintable(path));
    }
    return success;
}

/** \brief  Format an actor's name into a filename that doesn't contain characters that need escaping.
 *  \param  QString actorName:  Name of Actor
 *  \return QString fullPath:   Absolute path to the file.
 */
QString getHeadshotName(const QString &actorName){
    QString name_copy = QString("%1").arg(actorName);
    QString filename = name_copy.trimmed().replace(QRegularExpression("[\\s\\.']"), "_").toLower();
    QString fullPath = QString("%1/%2.jpg").arg(findHeadshotLocation()).arg(filename);
    return fullPath;
}
QString getHeadshotThumbnailName(const QString &actorName){
    QString name_copy = QString("%1").arg(actorName);
    QString filename = name_copy.trimmed().replace(QRegularExpression("[\\s\\.']"), "_").toLower();
    QString fullPath = QString("%1/%2_thumb%3.jpg").arg(findHeadshotLocation()).arg(filename).arg(ACTOR_LIST_PHOTO_HEIGHT);
    return fullPath;
}


/** \brief  Check the filesystem for the existence of the headshot for the actor with the name provided.
 *  \param  QString actorName:  name of actor.
 *  \return QString filename:   Path to file if it exists, Empty string if it doesn't exist.
 */
bool headshotDownloaded(const QString &actorName){
    bool downloaded = false;
    QString filepath = getHeadshotName(actorName);
    QFile f(filepath);
    //qDebug("Searching for %s's headshot (filename: %s)...", qPrintable(actorName), qPrintable(filepath));
    if (f.exists()){
        downloaded = (QFileInfo(f).size() > 200);
    }
    return downloaded;
}

bool thumbnailExists(const QString actorName){
    bool exists = false;
    QString filepath = getHeadshotThumbnailName(actorName);
    QFile f(filepath);
    //qDebug("Searching for %s's headshot (filename: %s)...", qPrintable(actorName), qPrintable(filepath));
    if (f.exists()){
        exists = (QFileInfo(f).size() > 200);
    }
    return exists;
}

QString getProfilePhoto(const QString &actorName){
    QString location = DEFAULT_PROFILE_PHOTO;
    if (headshotDownloaded(actorName)){
        location = getHeadshotName(actorName);
    }
    return location;
}

QString getHeadshotThumbnail(const QString &actorName){
    QString location = DEFAULT_PROFILE_PHOTO_THUMB;
    QString thumbnailName = getHeadshotThumbnailName(actorName);
    QFile small(thumbnailName);
    if (!small.exists() || small.size() < 200){
        QString headshotName = getHeadshotName(actorName);
        QFile big(headshotName);
        if (!big.exists() || big.size() < 200){
            return location;
        } else {
            QImage thumb = scaleImage(headshotName, ACTOR_LIST_PHOTO_HEIGHT);
            QImageWriter writer(thumbnailName);
            if(writer.write(thumb)){
                location = thumbnailName;
                qDebug("Created new thumbnail for '%s'", qPrintable(actorName));
            } else {
                qWarning("Error creating Thumbnail, '%s'", qPrintable(thumbnailName));
            }
        }
    } else {
        location = getHeadshotThumbnailName(actorName);
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
