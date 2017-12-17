#ifndef SCENE_H
#define SCENE_H
#include "genericfunctions.h"
#include <QDate>
#include <QDateTime>
#include <QSqlRecord>
#include <QString>
#include <QStringList>
#include <QVector>
#include "FilePath.h"
typedef QVector<QSharedPointer<Scene>> SceneList;

class Scene
{

private:
    FilePath file;
    double length;
    int height, width, size, sceneNumber;
    QDate added, released, opened;
    QString title, company, series, url;
    QStringList actors, tags;
    class Rating rating;
    QVector<int> ages;

public:
    Scene   (void);
    Scene   (FilePath);
    Scene   (QSqlRecord);
    ~Scene  (void);
    friend bool     hasScene(const Scene s);
    friend Scene duplicate(const Scene &s);
    friend bool    sqlInsertScene(const Scene *s, QString &query, QStringList &list);
    friend bool    sqlUpdateScene(const Scene *s, QString &query, QStringList &list);
    QString sqlInsert(void);
    QString sqlUpdate(void);
    bool    sqlInsert(QString &query, QStringList &list);
    bool    sqlUpdate(QString &query, QStringList &list);
    bool    inDatabase(void);
    void    addActor    (QString a, int i=0);
    void    setAge      (int, int);
    void    setAge      (QString, int);
    int     getAge      (QString name);
    bool    exists      (void)          { return file.exists(); }
    // Getters
    int         getSize     (void)      {   return size;        }
    int         getWidth    (void)      {   return width;       }
    int         getHeight   (void)      {   return height;      }
    int         getSceneNumber(void)    {   return sceneNumber; }
    double      getLength   (void)      {   return length;      }
    QString     getTitle    (void)      {   return title;       }
    QString     getCompany  (void)      {   return company;     }
    QString     getSeries   (void)      {   return series;      }
    QString     getRating   (void)      {   return rating;      }
    QStringList getActors   (void)      {   return actors;      }
    QStringList getTags     (void)      {   return tags;        }
    QDateTime   getOpened   (void)      {   return opened;      }
    QDateTime   getAdded    (void)      {   return added;       }
    QDate       getReleased (void)      {   return released;    }
    FilePath    getFile     (void)      {   return file;        }
    QString     getActor (int i=0)      {   return actors.at(i);    }
    int         getAge   (int i=0)      {   return ages.at(i);  }

    bool        hasActor(QString a)     {   return actors.contains(a);  }
    bool        hasTag(QString t)       {   return tags.contains(t);    }
    // Setters
    void    setSize     (int s)         {   this->size = s;     }
    void    setWidth    (int w)         {   this->width = w;    }
    void    setHeight   (int h)         {   this->height = h;   }
    void    setLength   (double l)      {   this->length = l;   }
    void    setTitle    (QString t)     {   this->title = t;    }
    void    setCompany  (QString c)     {  this->company = c;   }
    void    setSeries   (QString s)     {   this->series = s;   }
    void    setRating   (QString r)     {   this->rating = r;   }
    void    setActors   (QStringList a) {   this->actors = a;   }
    void    setTags     (QStringList t) {   this->tags = t;     }
    void    setOpened   (QDate d)       {   this->opened = d;   }
    void    setAdded    (QDate d)       {   this->added = d;    }
    void    setReleased (QDate d)       {   this->released = d; }
    void    setFile     (FilePath f)    {   this->file = f;     }
    void    setSceneNumber(int s)       {   this->sceneNumber = s;  }


};


#endif // SCENE_H
