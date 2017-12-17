#ifndef SCENEPARSER_H
#define SCENEPARSER_H
#include <QDate>
#include <QDateTime>
#include <QVector>
#include <QString>
#include <QStringList>
#define FEAT            "feat."
#define NAME_SEPERATOR  " - "
#define SPACE_REGEX     "[\\s\\t]*"

class sceneParser
{
private:
    class FilePath file;
    QString currPath, currName;
    QStringList tags, actors;
    QString title, company, series;
    int height, width, size, sceneNumber;
    double length;
    class Rating rating;
    QDate release;
    QDateTime accessed, created;
    QStringList bracketedData;
    void        bashScript        (class FilePath);
    QStringList parseActors       (QString);
    QStringList parseTags         (QString);
    QString     parseCompany      (QString);
    int         parseSceneNumber  (QString);
    QDate       parseDateReleased (QString);
public:
    sceneParser();
    sceneParser(class FilePath f);
    ~sceneParser();
    void        parse       (class FilePath);
    static QString sysCall  (QString);

    int         getSize()       {   return size;    }
    int         getWidth()      {   return width;   }
    int         getHeight()     {   return height;  }
    int         getSceneNumber(){   return sceneNumber; }
    double      getLength()     {   return length;  }
    QString     getTitle()      {   return title;   }
    QString     getCompany()    {   return company; }
    QString     getSeries()     {   return series;  }
    QString     getRating()     {   return rating;  }
    QStringList getActors()     {   return actors;  }
    QStringList getTags()       {   return tags;    }
    QDateTime   getAccessed()   {   return accessed;}
    QDateTime   getAdded()      {   return created; }
    QDate       getReleased()   {   return release; }
    class FilePath    getFile()       {   return file;    }

};

#endif // SCENEPARSER_H
