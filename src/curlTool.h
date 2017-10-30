#ifndef CURLTOOL_H
#define CURLTOOL_H
#include <QString>
#include <QStringList>
#define APP_DATA_PATH   "/Users/derby/Library/Application Support/SceneQuery"
#define IAFD_URL        "http://www.iafd.com/person.rme"
#define FREEONES_URL    "http://www.freeones.ca/html"
#define HEADSHOT_PATH   "headshots"
//enum CurlRequest { IAFD, Freeones, Photo, Filmography };
enum Website { IAFD, Freeones };
class curlTool
{
public:
    curlTool();
    ~curlTool();
    class FilePath  getPhoto        (QString, QString);
    class Biography getBiography    (class Actor &a);
    class Biography freeones        (QString);
    static QString  request(QString url);
    bool            downloadHeadshot(QString);
    class Biography iafd            (QString);

private:
    QString         userAppData;
    QString         datalocation;
    QString         headshotPath;
   // QString         query           (QString name, QStringList keys, QStringList values, CurlRequest type);

    QString         getHTML         (Website w, QString name);
    bool            wget            (QString, QString);
    bool            getDimensions   (QString name, class Height &height, int &weight);
    QString         bioSearchIAFD   (QString html, QString key);
    QString         bioSearchFO     (QString html, QString key);
};
static size_t   WriteCallback   (void *, size_t, size_t, void *);
QString         headshotName    (QString name);
#endif // CURLTOOL_H
