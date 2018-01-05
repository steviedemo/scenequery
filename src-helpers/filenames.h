#ifndef FILENAMES_H
#define FILENAMES_H
#include <QString>
#include "definitions.h"
QString getProfilePhoto(QString actorName);
QString getHeadshotName(QString actorName);
bool    headshotDownloaded(QString actorName);
bool    thumbnailExists(ScenePtr s);
QString getThumbnailName(ScenePtr, int);
#endif // FILENAMES_H
