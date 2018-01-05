#include "config.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QStandardPaths>
#include <QHostInfo>

bool makeDirectories(){
    bool error = false;
    QString dataPath = findDataLocation();
    QString headshotPath = findHeadshotLocation();
    QString thumbnailPath = findThumbnailLocation();

    if (!QDir(dataPath).exists()){
        qDebug("Creating Data directory...");
        if (!QDir::home().mkdir(DATA_FOLDER)){
            qWarning("Error Creating %s", qPrintable(dataPath));
            error = true;
        } else {
            qDebug("Successfully Created %s", qPrintable(dataPath));
        }
    }
    QString *i;
    if (!error){
        QDir dataFolder(dataPath);
        if (!QDir(headshotPath).exists()){
            qDebug("Creating Headshot Folder...");
            if (!dataFolder.mkdir(HEADSHOT_FOLDER)){
                qWarning("Error Creating %s", qPrintable(headshotPath));
                error = true;
            } else {
                qDebug("Successfully Created %s", qPrintable(headshotPath));
            }
        }
        if (!QDir(thumbnailPath).exists()){
            qDebug("Creating Thumbnail Path...");
            if (!dataFolder.mkdir(THUMBNAIL_FOLDER)){
                qWarning("Error Creating %s", qPrintable(thumbnailPath));
                error = true;
            } else {
                qDebug("Successfully Created %s", qPrintable(thumbnailPath));
            }
        }
    }
    return !error;
}

QString findHeadshotLocation(){
    return QString("%1/%2/%3").arg(QDir::homePath()).arg(DATA_FOLDER).arg(HEADSHOT_FOLDER);
}

QString findThumbnailLocation(){
    return QString("%1/%2/%3").arg(QDir::homePath()).arg(DATA_FOLDER).arg(THUMBNAIL_FOLDER);
}

QString findDataLocation(){
    return QString("%1/%2").arg(QDir::homePath()).arg(DATA_FOLDER);
}
