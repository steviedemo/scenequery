#ifndef FILENAMES_H
#define FILENAMES_H
#include <QString>
#include "definitions.h"
#include <QPair>
bool isEmpty(QPair<QString,QString>);
QPair<QString, QString> splitAbsolutePath(QString path);
QString toAbsolutePath(QPair<QString,QString> file);
QString getExtension(QString filename);
QString getProfilePhoto(QString actorName);
QString getHeadshotName(QString actorName);
bool    headshotDownloaded(QString actorName);
bool    thumbnailExists(ScenePtr s);
QString getThumbnailName(ScenePtr, int);
#endif // FILENAMES_H
