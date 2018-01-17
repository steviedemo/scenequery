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
#include "Rating.h"
#include <pqxx/pqxx>
#include "Entry.h"
#include <QPair>
bool operator==(const Scene &s1, const Scene &s2);
bool operator ==(const ScenePtr &scene1, const ScenePtr &scene2);
//bool operator !=(const ScenePtr &scene1, const ScenePtr &scene2);
class Scene : public Entry
{

private:
    long long int ID;
    QPair<QString,QString> file;
    QTime length;
    int height, width, sceneNumber;
    qint64 size;
    QDate added, released, opened;
    QString title, company, series, url;
    QStringList actors, tags;
    Rating rating;
    QVector<int> ages;
    void fromParser(class sceneParser p);
    ItemPtr itemTitle, itemActors, itemCompany, itemRating, itemDate, itemQuality, itemLength, itemSize, itemFeaturedActors, itemPath;
    bool displayBuilt;
    QByteArray md5sum;
    ItemList displayRow;
public:    
    QString dateString;
//    struct RowData{
//        QString filename, filepath, title, company, series, quality;
//        QString rating, mainActor, featured, size, date, length;
//        RowData():filename(""), filepath(""),title(""),company(""),series(""), quality(""),
//            rating(""), mainActor(""), featured(""), size(""), date(""), length(""){}
//    };
    Scene   (void);
    Scene   (QString);
    Scene   (QSqlRecord);
    Scene   (class sceneParser);
    Scene   (pqxx::result::const_iterator record);
    Scene   (const Scene &s);
    ~Scene  (void);
    friend bool     hasScene(const Scene s);
    friend Scene duplicate(const Scene &s);
    friend bool operator ==(const Scene &s1, const Scene &s2);
    Query   toQuery() const;
    void    fromRecord(pqxx::result::const_iterator record);
    int     entrySize();
    QList<QStandardItem *> buildQStandardItem();
    void    updateQStandardItem();
    QList<QStandardItem *> getQStandardItem();
    ItemList getItemList();

    bool    sqlInsert(QString &query, QStringList &list) const;
    bool    sqlUpdate(QString &query, QStringList &list) const;
    bool    inDatabase(void);
    void    addActor    (QString a);
    void    removeActor (QString a);
    void    renameActor (QString oldName, QString newName);
    void    setAge      (int, int);
    void    setAge      (QString, int);
    int     getAge      (QString name);
    bool    hasDisplay  (void) ;
    bool    exists      (void) const;
    bool    equals      (const Scene &other) const;
    bool    equals      (const ScenePtr &other) const;
    // Getters
    int         getID           (void) const {   return ID;             }
    qint64      getSize         (void) const {   return size;           }
    int         getWidth        (void) const {   return width;          }
    int         getHeight       (void) const {   return height;         }
    int         getSceneNumber  (void) const {   return sceneNumber;    }
    QTime       getLength       (void) const {   return length;         }
    QString     getTitle        (void) const {   return title;          }
    QString     getCompany      (void) const {   return company;        }
    QString     getSeries       (void) const {   return series;         }
    Rating      getRating       (void) const {   return rating;         }
    QStringList getActors       (void) const {   return actors;         }
    QStringList getTags         (void) const {   return tags;           }
    QString     tagString       (void) const;
    QDate       getOpened       (void) const {   return opened;         }
    QDate       getAdded        (void) const {   return added;          }
    QDate       getReleased     (void) const;
    QString     getReleaseString(void) const;
    QPair<QString,QString>  getFile (void) const {   return file;       }
    QString     getFullpath     (void) const {   return QString("%1/%2").arg(file.first).arg(file.second); }
    QString     getFilename     (void) const {   return file.second;    }
    QString     getFolder       (void) const {   return file.first;     }
    QString     getActor     (int i=0) const {
        if (actors.size() > (i))
            return actors.at(i);
        else
            return "";
    }
    int  getAge  (int i=0) const {
        if (ages.size() > i)
            return ages.at(i);
        else
            return 0;
    }

    bool        hasActor(QString a) const {   return actors.contains(a);  }
    bool        hasTag  (QString t) const {   return tags.contains(t);    }
    // Setters
    void    setSize     (const qint64 &s)      {   this->size = s;         }
    void    setWidth    (const int &w)         {   this->width = w;        }
    void    setHeight   (const int &h)         {   this->height = h;       }
    void    setLength   (const QTime &t);
    void    setTitle    (const QString &t);
    void    setCompany  (const QString &c);
    void    setSeries   (const QString &s);
    void    setRating   (const Rating &r)      {   this->rating = r;       }
    void    setRating   (const QString &r)     {   this->rating = Rating(r);   }
//    void    setRating   (int i)      {   this->rating = Rating(d);   }
    void    setActors   (const QStringList &a) {   this->actors = a;       }
    void    setTags     (const QStringList &t) {   this->tags = t;         }
    void    setOpened   (const QDate &d)       {   this->opened = d;       }
    void    setAdded    (const QDate &d)       {   this->added = d;        }
    void    setReleased (const QDate &d);
    void    setFile     (const QString &absolutePath);
    void    setFile     (const QPair<QString,QString> &file)   {   this->file = file;  }
    void    setSceneNumber(const int &s)       {   this->sceneNumber = s;  }

};




#endif // SCENE_H
