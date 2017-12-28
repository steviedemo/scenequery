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
#include "Rating.h"
#include <pqxx/pqxx>
#include "Entry.h"
class Scene : public Entry
{

private:
    long long int ID;
    FilePath file;
    double length;
    int height, width, size, sceneNumber;
    QDate added, released, opened;
    QString title, company, series, url;
    QStringList actors, tags;
    Rating rating;
    QVector<int> ages;
    void fromParser(class sceneParser p);
public:
    Scene   (void);
    Scene   (FilePath);
    Scene   (QSqlRecord);
    Scene   (class sceneParser);
    Scene   (pqxx::result::const_iterator &it);
    ~Scene  (void);
    friend bool     hasScene(const Scene s);
    friend Scene duplicate(const Scene &s);

    Query   toQuery() const;
    void    fromRecord(pqxx::result::const_iterator &record);
    int     entrySize();
    QList<QStandardItem *> buildQStandardItem();
    void    updateQStandardItem();
    bool    sqlInsert(QString &query, QStringList &list) const;
    bool    sqlUpdate(QString &query, QStringList &list) const;
    bool    inDatabase(void);
    void    addActor    (QString a, int i=0);
    void    setAge      (int, int);
    void    setAge      (QString, int);
    int     getAge      (QString name);
    bool    exists      (void)          { return file.exists(); }
    // Getters
    int         getID       (void)      {   return ID;          }
    int         getSize     (void)      {   return size;        }
    int         getWidth    (void)      {   return width;       }
    int         getHeight   (void)      {   return height;      }
    int         getSceneNumber(void)    {   return sceneNumber; }
    double      getLength   (void)      {   return length;      }
    QString     getTitle    (void)      {   return title;       }
    QString     getCompany  (void)      {   return company;     }
    QString     getSeries   (void)      {   return series;      }
    Rating      getRating   (void)      {   return rating;      }
    QStringList getActors   (void)      {   return actors;      }
    QStringList getTags     (void)      {   return tags;        }
    QString     tagString   (void) const;
    QDate       getOpened   (void)      {   return opened;      }
    QDate       getAdded    (void)      {   return added;       }
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
    void    setRating   (Rating r)      {   this->rating = r;   }
    void    setRating   (QString r)     {   this->rating = Rating(r);   }
    void    setRating   (double d)      {   this->rating = Rating(d);   }
    void    setActors   (QStringList a) {   this->actors = a;   }
    void    setTags     (QStringList t) {   this->tags = t;     }
    void    setOpened   (QDate d)       {   this->opened = d;   }
    void    setAdded    (QDate d)       {   this->added = d;    }
    void    setReleased (QDate d)       {   this->released = d; }
    void    setFile     (FilePath f)    {   this->file = f;     }
    void    setSceneNumber(int s)       {   this->sceneNumber = s;  }


};


#endif // SCENE_H
