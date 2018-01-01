#ifndef SCENE_H
#define SCENE_H
#include <QByteArray>
#include "definitions.h"
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

bool operator==(const Scene &s1, const Scene &s2);
bool operator ==(const ScenePtr &scene1, const ScenePtr &scene2);
//bool operator !=(const ScenePtr &scene1, const ScenePtr &scene2);
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
    ItemPtr itemTitle, itemActors, itemCompany, itemRating, itemDate, itemQuality, itemLength, itemSize, itemFeaturedActors;
    bool displayBuilt;
    QByteArray md5sum;
    ItemList displayRow;
public:    
    struct RowData{
        QString filename, filepath, title, company, series, quality;
        QString rating, mainActor, featured, size, date, length;
        RowData():filename(""), filepath(""),title(""),company(""),series(""), quality(""),
            rating(""), mainActor(""), featured(""), size(""), date(""), length(""){}
    };
    Scene   (void);
    Scene   (FilePath);
    Scene   (QSqlRecord);
    Scene   (class sceneParser);
    Scene   (pqxx::result::const_iterator &it);
    ~Scene  (void);
    friend bool     hasScene(const Scene s);
    friend Scene duplicate(const Scene &s);
    friend bool operator ==(const Scene &s1, const Scene &s2);
    Query   toQuery() const;
    void    fromRecord(pqxx::result::const_iterator &record);
    int     entrySize();
    QList<QStandardItem *> buildQStandardItem();
    void    updateQStandardItem();
    QList<QStandardItem *> getQStandardItem();
    ItemList getItemList();
    RowData getRowData();

    bool    sqlInsert(QString &query, QStringList &list) const;
    bool    sqlUpdate(QString &query, QStringList &list) const;
    bool    inDatabase(void);
    void    addActor    (QString a, int i=0);
    void    setAge      (int, int);
    void    setAge      (QString, int);
    int     getAge      (QString name);
    bool    hasDisplay  (void) ;
    bool    exists      (void) const { return file.exists(); }
    bool    equals      (const Scene &other) const;
    bool    equals      (const ScenePtr &other) const;
    // Getters
    int         getID           (void) const {   return ID;          }
    int         getSize         (void) const {   return size;        }
    int         getWidth        (void) const {   return width;       }
    int         getHeight       (void) const {   return height;      }
    int         getSceneNumber  (void) const {   return sceneNumber; }
    double      getLength       (void) const {   return length;      }
    QString     getTitle        (void) const {   return title;       }
    QString     getCompany      (void) const {   return company;     }
    QString     getSeries       (void) const {   return series;      }
    Rating      getRating       (void) const {   return rating;      }
    QStringList getActors       (void) const {   return actors;      }
    QStringList getTags         (void) const {   return tags;        }
    QString     tagString       (void) const;
    QDate       getOpened       (void) const {   return opened;      }
    QDate       getAdded        (void) const {   return added;       }
    QDate       getReleased     (void) const {   return released;    }
    FilePath    getFile         (void) const {   return file;        }
    QString     getFilepath     (void) const {   return file.absolutePath(); }
    QString     getActor     (int i=0) const {   return actors.at(i);}
    int         getAge       (int i=0) const {   return ages.at(i);  }
    QByteArray  getMd5sum       (void) const {   return md5sum;      }

    bool        hasActor(QString a) const {   return actors.contains(a);  }
    bool        hasTag  (QString t) const {   return tags.contains(t);    }
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
//    void    setRating   (int i)      {   this->rating = Rating(d);   }
    void    setActors   (QStringList a) {   this->actors = a;   }
    void    setTags     (QStringList t) {   this->tags = t;     }
    void    setOpened   (QDate d)       {   this->opened = d;   }
    void    setAdded    (QDate d)       {   this->added = d;    }
    void    setReleased (QDate d)       {   this->released = d; }
    void    setFile     (FilePath f)    {   this->file = f;     }
    void    setSceneNumber(int s)       {   this->sceneNumber = s;  }
    void    setMd5sum   (QByteArray b)  {   this->md5sum = b;   }
    void    setMd5sum   (void);


};




#endif // SCENE_H
