#include "config.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QStandardPaths>
config::config() :
    configName("SceneQuery.plist")
{
    configLocation = QStandardPaths::locate(QStandardPaths::ConfigLocation, configName);
    if (configLocation.isEmpty()){
       // configLocation =
    }
    QFile file("");
}
