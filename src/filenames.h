#ifndef FILENAMES_H
#define FILENAMES_H
#include "definitions.h"
#include <QPair>
#include <QString>
bool isEmpty(const QPair<QString,QString> &);
QPair<QString, QString> splitAbsolutePath(const QString path);
bool splitAbsolutePath(const QString path, QString &filepath, QString &filename);
QString toAbsolutePath(const QPair<QString,QString> &file);
QString getExtension(const QString &filename);
QString getProfilePhoto(const QString &actorName);
QString getHeadshotName(const QString &actorName);
QString getHeadshotThumbnailName(const QString &actorName);
QString getHeadshotThumbnail(const QString &actorName);
bool    writeHeadshotThumbnail(const QString &headshotFilename, const QString &thumbnailFilename);
bool    writeHeadshotThumbnail(const QString &headshotFilename);
bool    headshotDownloaded(const QString &actorName);
bool    thumbnailExists(const QString &actorName);
bool    thumbnailExists(ScenePtr s);
QString getThumbnailName(ScenePtr, int);
#endif // FILENAMES_H
