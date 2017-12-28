#ifndef ACTOR_H
#define ACTOR_H
#include <QDate>
#include <QString>
#include <QSqlRecord>
#include <QSharedPointer>
#include <QStandardItem>
#include <QVector>
// Custom Objects
#include "Biography.h"
#include "FilePath.h"
#include "Entry.h"
#include <pqxx/result>
#define ACTOR_LIST_PHOTO_HEIGHT 40
typedef QVector<QSharedPointer<class Actor>> ActorList;

class Actor : public Entry
{
private:
    QString name;
    Biography bio;
    double dataUsage;
    class FilePath headshot;
    QSharedPointer<QStandardItem> itemName, itemAge, itemPhoto, itemHair, itemEthnicity;
    QImage photo;
    bool displayItemCreated;
    void setup();

public:
    Actor(QString name="");
    Actor(const Actor &a);
    Actor(QString name, Biography bio, QString headshot);
    //Actor(QSqlRecord);
    Actor(pqxx::result::const_iterator &i);
    ~Actor();
    void fromRecord(pqxx::result::const_iterator &record);
    int  entrySize();
    QList<QStandardItem *> buildQStandardItem();
    void updateQStandardItem();
    QSharedPointer<QStandardItem> getNameItem();
    bool updateBio();
    bool hasBio();
    // Operators
    Actor operator =  (Actor &other);
    bool  operator == (Actor &other) const;
    bool  operator >  (Actor &other) const;
    bool  operator <  (Actor &other) const;
    // Processing
    Query   toQuery (void) const;
    bool    inDatabase  (void);
    QString headshotTitle(void);
    void    setProfilePicture(QImage image);
    // Setters
    void    setHeadshot (FilePath f);
    void    setHeadshot (QString s);

    void    setBio      (Biography b)   {   this->bio = b;                  }
    void    setName     (QString n)     {   this->name = n; bio.setName(n); }
    void    setWeight   (int i)         {   this->bio.setWeight(i);         }
    void    setHeight   (Height h)      {   this->bio.setHeight(h);         }
    void    setAliases  (QString s)     {   this->bio.setAliases(s);        }
    void    setCity     (QString s)     {   this->bio.setCity(s);           }
    void    setBirthday (QDate d)       {   this->bio.setBirthday(d);       }
    void    setEthnicity(QString s)     {   this->bio.setEthnicity(s);      }
    void    setEyes     (QString s)     {   this->bio.setEyeColor(s);       }
    void    setHair     (QString s)     {   this->bio.setHairColor(s);      }
    void    setMeasurements(QString s)  {   this->bio.setMeasurements(s);   }
    void    setNationality(QString s)   {   this->bio.setNationality(s);    }
    void    setPiercings(QString s)     {   this->bio.setPiercings(s);      }
    void    setTattoos  (QString s)     {   this->bio.setTattoos(s);        }
    void    setFakeBoobs(bool b)        {   this->bio.fakeBoobs = b;        }
    void    setCareerStart(QDate d)     {   this->bio.careerStart=d;        }
    void    setCareerEnd(QDate d)       {   this->bio.careerEnd = d;    this->bio.retired = true;   }

    // Getters
    QString     getName         (void)  {   return this->name;                  }
    Biography   getBio        	(void)  {   return this->bio;                   }
    int			getWeight       (void)  {   return this->bio.getWeight();       }
   // int         getRetirement   (void)  {   return this->bio.retirement;      }
    Height      getHeight   	(void)  {   return this->bio.getHeight();       }
    QString		getAliases    	(void)  {   return this->bio.getAliases();      }
    QString		getCity         (void)  {   return this->bio.getCity();         }
    QDate       getBirthday		(void)  {   return this->bio.getBirthday();     }
    QString		getNationality  (void)  {   return this->bio.getNationality();  }
    QString		getMeasurements	(void)  {   return this->bio.getMeasurements(); }
    QString		getHairColor	(void)  {   return this->bio.getHairColor();    }
    QString		getEyeColor		(void)  {   return this->bio.getEyeColor();     }
    QString     getTattoos  	(void)  {   return this->bio.getTattoos();      }
    QString		getPiercings    (void)  {   return this->bio.getPiercings();    }
    FilePath    getHeadshot    	(void)  {   return this->headshot;              }

};

#endif // ACTOR_H
