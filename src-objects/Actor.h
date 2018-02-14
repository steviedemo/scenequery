#ifndef ACTOR_H
#define ACTOR_H
#include "definitions.h"
#include "filenames.h"
#include <QDate>
#include <QString>
#include <QSqlRecord>
#include <QSharedPointer>
#include <QStandardItem>
#include <QVector>
// Custom Objects
#include "Biography.h"
#include "Entry.h"
#include "SceneList.h"
#include <pqxx/result>

typedef QVector<QSharedPointer<class Actor>> ActorList;

class Actor : public Entry
{
private:
    QString name;
    Biography bio;
    double dataUsage;
    QString photoPath;
    QStandardItem *itemName, *itemAge, *itemPhoto, *itemHair, *itemEthnicity;
    QStandardItem *itemSceneCount, *itemBioSize, *itemHeight, *itemWeight, *itemTattoos, *itemPiercings;
    int sceneCount;
    bool displayItemCreated, photoItemCreated;
    SceneList sceneList;
    QVariant profilePhoto;
    bool favourite;
    QList <QStandardItem*> row;
    void setup() { dataUsage = 0.0; sceneCount = 0; sceneList = {}; }
    void updateSceneCountItem(){
        if (!itemSceneCount){
            itemSceneCount = new QStandardItem();
        }
        itemSceneCount->setData(QVariant(sceneCount), Qt::DecorationRole);
    }
public slots:
public:
    void updateQStandardItem();
    Actor(QString name=""): Entry(), name(name), bio(name), photoPath(getProfilePhoto(name)){
        setup();
    }
    Actor(QString name, Biography bio, QString headshot) :
        Entry(), name(name), bio(bio), photoPath(headshot){  setup();    }
    Actor(const Actor &a) :
        Entry(), name(a.name), bio(a.bio), dataUsage(a.dataUsage), photoPath(a.photoPath){ setup(); }
    Actor(pqxx::result::const_iterator &i) :
        Entry(){   fromRecord(i);    }
    //~Actor();
    bool    save() const;
    void                    fromRecord          (pqxx::result::const_iterator record);
    QList<QStandardItem *>  buildQStandardItem  (void);
    QList<QStandardItem *>  getQStandardItem    (void) { return row;           }
    QStandardItem *         getNameItem         (void) const { return itemName;      }
    bool    updateBio   (void);
    void    setBio      (const Biography &b){ this->bio.copy(b);  }
    int     size        (void)  { return entrySize(); }
    int     entrySize   (void)  { return (bio.size() + (name.isEmpty() ? 0 : 1) + (usingDefaultPhoto() ? 0 : 1)); }
    bool    hasBio      (void)  { return (bio.size() > 3);    }
    bool    isEmpty     (void)  const       { return name.isEmpty();      }
    // Operators
    //Actor operator =    (Actor &other);
    bool  operator ==   (Actor &other) const { return name == other.getName();    }
    bool  operator >    (Actor &other) const { return name > other.getName();     }
    bool  operator <    (Actor &other) const { return name < other.getName();     }
    // Processing
    bool    inDatabase              (void);
    Query   toQuery                 (void) const;
    QString headshotTitle           (void) const        {   return photoPath;       }
    bool    usingDefaultPhoto       (void) const        {   return (photoPath == DEFAULT_PROFILE_PHOTO);   }
    void    setHeadshot             (const QString s)   {   this->photoPath = s;    }
    void    setScaledProfilePhoto   (const QVariant v)  {   this->profilePhoto = v; }
    void    setDefaultHeadshot();
    void    deleteHeadshot();
    // Setters
    void    setScenes       (class SceneList list) {   sceneList = list;  sceneCount = list.size(); updateSceneCountItem(); }
    void    setSceneCount   (const int &i)      {   sceneCount = i; updateSceneCountItem(); }
    void    addScene        (void)              {   sceneCount++;   updateSceneCountItem(); }
    void    addScene        (const ScenePtr s)  {   sceneList << s; sceneCount++; updateSceneCountItem(); }
    void    setName         (QString n)         {   this->name = n; bio.setName(n); }
    void    setWeight       (int i)             {   this->bio.setWeight(i);         }
    void    setHeight       (Height h)          {   this->bio.setHeight(h);         }
    void    setAliases      (QString s)         {   this->bio.setAliases(s);        }
    void    setCity         (QString s)         {   this->bio.setCity(s);           }
    void    setBirthday     (QDate d)           {   this->bio.setBirthday(d);       }
    void    setEthnicity    (QString s)         {   this->bio.setEthnicity(s);      }
    void    setEyes         (QString s)         {   this->bio.setEyeColor(s);       }
    void    setHair         (QString s)         {   this->bio.setHairColor(s);      }
    void    setMeasurements (QString s)         {   this->bio.setMeasurements(s);   }
    void    setNationality  (QString s)         {   this->bio.setNationality(s);    }
    void    setPiercings    (QString s)         {   this->bio.setPiercings(s);      }
    void    setTattoos      (QString s)         {   this->bio.setTattoos(s);        }
    void    setFakeBoobs    (bool b)            {   this->bio.fakeBoobs = b;        }
    void    setCareerStart  (QDate d)           {   this->bio.careerStart=d;        }
    void    setCareerEnd    (QDate d)           {   this->bio.careerEnd = d;    this->bio.retired = true;   }

    // Getters
    int         getSceneCount   (void)  const {   return this->sceneCount;            }
    QString     getName         (void)  const {   return this->name;                  }
    Biography   getBio        	(void)  const {   return this->bio;                   }
    int			getWeight       (void)  const {   return this->bio.getWeight();       }
   // int         getRetirement   (void)  {   return this->bio.retirement;      }
    Height      getHeight   	(void)  const {   return this->bio.getHeight();       }
    QString		getAliases    	(void)  const {   return this->bio.getAliases();      }
    QString		getCity         (void)  const {   return this->bio.getCity();         }
    QString     getEthnicity    (void)  const {   return this->bio.getEthnicity();    }
    QDate       getBirthday		(void)  const {   return this->bio.getBirthday();     }
    QString		getNationality  (void)  const {   return this->bio.getNationality();  }
    QString		getMeasurements	(void)  const {   return this->bio.getMeasurements(); }
    QString		getHairColor	(void)  const {   return this->bio.getHairColor();    }
    QString		getEyeColor		(void)  const {   return this->bio.getEyeColor();     }
    QString     getTattoos  	(void)  const {   return this->bio.getTattoos();      }
    QString		getPiercings    (void)  const {   return this->bio.getPiercings();    }
    QString     getHeadshot     (void)  const {   return this->photoPath;             }
};

#endif // ACTOR_H
