#ifndef CONFIG_H
#define CONFIG_H
#include <QString>

class config
{
public:
    config();
    QString dataLocation();
    QString headshotLocation();
    QString databaseLocation();
    QString makePaths();

private:
    QString configName;
    QString configLocation;
    QString headshotPath, dbPath, dataPath;


};

#endif // CONFIG_H
