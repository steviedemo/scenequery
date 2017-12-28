#ifndef SCENEPARSER_H
#define SCENEPARSER_H
#include <QDate>
#include <QDateTime>
#include <QVector>
#include <QString>
#include <QStringList>
#include "FilePath.h"
#include "Rating.h"
#define FEAT            "feat."
#define NAME_SEPERATOR  " - "
#define SPACE_REGEX     "[\\s\\t]*"

class sceneParser
{
private:
    FilePath file;
    bool parsed;
    QString currPath, currName;
    QStringList tags, actors;
    QString title, company, series;
    int height, width, size, sceneNumber;
    double length;
    Rating rating;
    QDate release;
    QDate accessed, created;
    QStringList bracketedData;
    void        bashScript        (class FilePath);
    QStringList parseActors       (QString);
    QStringList parseTags         (QString);
    QString     parseCompany      (QString);
    int         parseSceneNumber  (QString);
    QDate       parseDateReleased (QString);
    QString     parseTitle        (QString);
public:
    sceneParser();
    sceneParser(class FilePath f);
    ~sceneParser();
    void        parse       (void);
    void        parse       (class FilePath);
    static QString sysCall  (QString);
    bool        isParsed()      {   return parsed;  }
    bool        isEmpty()       {   return file.isEmpty();  }
    int         getSize()       {   return size;    }
    int         getWidth()      {   return width;   }
    int         getHeight()     {   return height;  }
    int         getSceneNumber(){   return sceneNumber; }
    double      getLength()     {   return length;  }
    QString     getTitle()      {   return title;   }
    QString     getCompany()    {   return company; }
    QString     getSeries()     {   return series;  }
    Rating      getRating()     {   return rating;  }
    QStringList getActors()     {   return actors;  }
    QStringList getTags()       {   return tags;    }
    QDate       getAccessed()   {   return accessed;}
    QDate       getAdded()      {   return created; }
    QDate       getReleased()   {   return release; }
    class FilePath    getFile()       {   return file;    }

};

#define EXIF_COMMAND "data=$(exiftool \"$1\") \
    time=$(echo \"$data\" | grep \"Media Duration\" | awk '{print $4}') \
    width=$(echo \"$data\" | awk /\"Source Image Width\"/ '{print $5}') \
    if [[ ! -z $width ]]; then  \
        echo \"Width: \"$(echo \"$data\" | grep \"Source Image Width\" | awk '{print $5}')  \
    fi  \
    height=$(echo \"$data\" | awk /\"Source Image Height\"/ '{print $5}')   \
    if [[ ! -z $height ]]; then \
        echo \"Height: \"$(echo \"$data\" | grep \"Source Image Height\" | awk '{print $5}')    \
    fi  \
    min=$(echo \"$data\" | awk -F: '{print $2}')    \
    if [[ ! -z $min ]]; then    \
        echo \"Minutes: \"$(echo \"$time\" | awk -F: '{print $2}')  \
    fi  \
    sec=$(echo \"$data\" | awk -F; '{print $3}');   \
    if [[ ! -z $sec ]]; then    \
        echo \"Seconds: \"$(echo \"$time\" | awk -F: '{print $3}')  \
    fi"

#endif // SCENEPARSER_H
