#ifndef FILENAMES_H
#define FILENAMES_H
#include <QString>
#include "definitions.h"
#include <QPair>
bool isEmpty(const QPair<QString,QString> &);
QPair<QString, QString> splitAbsolutePath(const QString path);
bool splitAbsolutePath(const QString path, QString &filepath, QString &filename);
QString toAbsolutePath(const QPair<QString,QString> &file);
QString getExtension(const QString &filename);
QString getProfilePhoto(const QString &actorName);
QString getHeadshotName(const QString &actorName);
bool    headshotDownloaded(const QString &actorName);
bool    thumbnailExists(ScenePtr s);
QString getThumbnailName(ScenePtr, int);
#endif // FILENAMES_H
