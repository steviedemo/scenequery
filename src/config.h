#ifndef CONFIG_H
#define CONFIG_H
#include <QString>
#define DATA_FOLDER         ".scenequery"
#define HEADSHOT_FOLDER     "headshots"
#define THUMBNAIL_FOLDER    "thumbnails"
bool    makeDirectories();
QString findDataLocation();
QString findHeadshotLocation();
QString findThumbnailLocation();

#endif // CONFIG_H
