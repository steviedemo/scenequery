#ifndef ACTOR_H
#define ACTOR_H
#include <QDate>
#include <QString>
#include <QSqlRecord>
#include <QSharedPointer>
#include <QVector>
// Custom Objects
#include "Biography.h"
#include "Height.h"
#include "FilePath.h"

typedef QVector<QSharedPointer<class Actor>> ActorList;

class Actor
{
private:
    Biography bio;
    QString name;
    class FilePath headshot;
    double dataUsage;

public:
    Actor();
    Actor(QString n) : Actor(){ this->name = n; }
    Actor(const Actor &a);
    Actor(QSqlRecord);
    ~Actor();

    // Operators
    Actor operator =(Actor a);
    friend bool operator == (const Actor &a, const Actor &b)  {   return a.name == b.name;   }
    friend bool operator <  (const Actor &a, const Actor &b)  {   return a.name <  b.name;   }
    friend bool operator >  (const Actor &a, const Actor &b)  {   return a.name >  b.name;   }
    friend bool operator != (const Actor &a, const Actor &b)  {   return a.name != b.name;   }

    // Processing
    QString sqlInsert   (void);
    QString sqlUpdate   (void);
    bool    sqlInsert   (QString &query, QStringList &list);
    bool    sqlUpdate   (QString &query, QStringList &list);
    bool    inDatabase  (void);
    // Setters
    void    setName     (QString n)     {   this->name = n;         }
    void    setBio      (Biography b)   {   this->bio = b;          }
    void    setHeadshot (FilePath f)    {   this->headshot = f;     }
    void    setWeight   (int i)         {   this->bio.weight = i;   }
    void    setHeight   (Height h)      {   this->bio.height = h;   }
    void    setAliases  (QString s)     {   this->bio.aliases = s;  }
    void    setCity     (QString s)     {   this->bio.city = s;     }
    void    setBirthday (QDate d)       {   this->bio.birthdate = d;}
    void    setNationality(QString s)   {   this->bio.nationality=s;}
    void    setEthnicity(QString s)     {   this->bio.ethnicity = s;}
    void    setHair     (QString s)     {   this->bio.hair = s;     }
    void    setEyes     (QString s)     {   this->bio.eyes = s;     }
    void    setTattoos  (QString s)     {   this->bio.tattoos = s;  }
    void    setPiercings(QString s)     {   this->bio.piercings = s;}
    void    setFakeBoobs(bool b)        {   this->bio.fakeBoobs = b;}
    void    setCareerStart(QDate d)     {   this->bio.careerStart=d;}
    void    setCareerEnd(QDate d)       {   this->bio.careerEnd = d;    this->bio.retired = true;   }



    // Getters
    QString     getName         (void)  {   return this->name;              }
    Biography   getBio        	(void)  {   return this->bio;               }
    int			getWeight       (void)  {   return this->bio.weight;        }
   // int         getRetirement   (void)  {   return this->bio.retirement;    }
    Height      getHeight   	(void)  {   return this->bio.height;        }
    QString		getAliases    	(void)  {   return this->bio.aliases;       }
    QString		getCity         (void)  {   return this->bio.city;          }
    QDate       getBirthday		(void)  {   return this->bio.birthdate;     }
    QString		getNationality  (void)  {   return this->bio.nationality;   }
    QString		getMeasurements	(void)  {   return this->bio.measurements;  }
    QString		getHairColor	(void)  {   return this->bio.hair;          }
    QString		getEyeColor		(void)  {   return this->bio.eyes;          }
    QString     getTattoos  	(void)  {   return this->bio.tattoos;       }
    QString		getPiercings    (void)  {   return this->bio.piercings;     }
    FilePath    getHeadshot    	(void)  {   return this->headshot;          }

};

#endif // ACTOR_H
