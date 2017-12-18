#include "Actor.h"
#include "sql.h"
#include <QRegularExpression>
#include <QStringList>
Actor::Actor(QString name):
    name(name){
    this->dataUsage = 0.0;
    this->headshot = FilePath();
}
Actor::Actor(const Actor &a){
    this->bio = a.bio;
    this->name = a.name;
    this->dataUsage = a.dataUsage;
    this->headshot = a.headshot;
}

Actor::Actor(QString actorName, Biography bio, QString headshot):
    name(actorName), bio(bio), dataUsage(0.0), headshot(headshot){
}

Actor::Actor(QSqlRecord r){
    Biography b;
    this->name      = r.value("name").toString();
    this->headshot  = r.value("photo").toString();
    b.aliases       = r.value("aliases").toString();
    b.birthdate     = QDate::fromString(r.value("birthday").toString(), "yyyy.MM.dd");
    b.city          = r.value("city").toString();
    b.nationality   = r.value("country").toString();
    b.ethnicity     = r.value("ethnicity").toString();
    b.height        = Height(r.value("height").toInt());
    b.weight        = r.value("weight").toInt();
    b.measurements  = r.value("measurements").toString();
    b.hair          = r.value("hair").toString();
    b.eyes          = r.value("eyes").toString();
    b.tattoos       = r.value("tattoos").toString();
    b.piercings     = r.value("piercings").toString();
    this->bio = b;
}

Actor::~Actor(){}

Actor Actor::operator =(Actor &obj){
    this->bio = obj.bio;
    this->name = obj.name;
    this->dataUsage = obj.dataUsage;
    this->headshot = obj.headshot;
    return *this;
}

bool Actor::operator < (Actor &other) const {  return (this->name < other.getName());  }
bool Actor::operator > (Actor &other) const {  return (this->name > other.getName());  }
bool Actor::operator ==(Actor &other) const {  return (this->name == other.getName()); }

/** \brief Make Curl Requests to IAFD & Freeones to get biographical Details */
bool Actor::updateBio(){
    return this->bio.update();
}

bool Actor::inDatabase(){
    QString queryString = QString("SELECT FROM %1 WHERE NAME = ?;").arg(ACTOR_DB);
    QStringList args;
    args << this->name;
    SQL sql;
    bool ok = false, success = false;
    QSharedPointer<QSqlQuery> q = sql.assembleQuery(queryString, args, ok);
    if (ok){
        success = sql.hasMatch(q.data());
    }
    return success;
}

//QString Actor::sqlInsert(void){
//    QString f("INSERT INTO ACTORS (NAME");
//    QString v = QString(") VALUES (%1").arg(sqlSafe(name));
//    sqlAppend(f, v, "ALIASES", sqlSafe(bio.aliases));
//    sqlAppend(f, v, "BIRTHDAY", sqlSafe(bio.birthdate));
//    sqlAppend(f, v, "CITY", sqlSafe(bio.city));
//    sqlAppend(f, v, "COUNTRY", sqlSafe(bio.nationality));
//    sqlAppend(f, v, "ETHNICITY", sqlSafe(bio.ethnicity));
//    sqlAppend(f, v, "HEIGHT", sqlSafe(bio.height.toString()));
//    sqlAppend(f, v, "WEIGHT", sqlSafe(bio.weight));
//    sqlAppend(f, v, "MEASUREMENTS", sqlSafe(bio.measurements));
//    sqlAppend(f, v, "HAIR", sqlSafe(bio.hair));
//    sqlAppend(f, v, "EYES", sqlSafe(bio.eyes));
//    sqlAppend(f, v, "TATTOOS", sqlSafe(bio.tattoos));
//    sqlAppend(f, v, "PIERCINGS", sqlSafe(bio.piercings));
//    sqlAppend(f, v, "PHOTO", sqlSafe(headshot));
//}

//QString Actor::sqlUpdate(void){
//    QString f("UPDATE actors SET ");
//    bool p = false;
//    sqlAppend(f, "ALIASES", sqlSafe(bio.aliases), p);
//    sqlAppend(f, "BIRTHDAY", sqlSafe(bio.birthdate), p);
//    sqlAppend(f, "CITY", sqlSafe(bio.city), p);
//    sqlAppend(f, "COUNTRY", sqlSafe(bio.nationality), p);
//    sqlAppend(f, "ETHNICITY", sqlSafe(bio.ethnicity), p);
//    sqlAppend(f, "HEIGHT", sqlSafe(bio.height.toString()), p);
//    sqlAppend(f, "WEIGHT", sqlSafe(bio.weight), p);
//    sqlAppend(f, "MEASUREMENTS", sqlSafe(bio.measurements), p);
//    sqlAppend(f, "HAIR", sqlSafe(bio.hair), p);
//    sqlAppend(f, "EYES", sqlSafe(bio.eyes), p);
//    sqlAppend(f, "TATTOOS", sqlSafe(bio.tattoos), p);
//    sqlAppend(f, "PIERCINGS", sqlSafe(bio.piercings), p);
//    sqlAppend(f, "PHOTO", sqlSafe(headshot), p);
//    if (p){
//        f.append(QString(" WHERE NAME = %1").arg(name));
//    }
//    return f;
//}

bool Actor::sqlInsert(QString &query, QStringList &list){
    list.clear();
    QString f ("INSERT INTO ACTORS (NAME"), v("?");
    list << sqlSafe(name);
    sqlAppend(f, v, list, "ALIASES",       sqlSafe(bio.aliases)         );
    sqlAppend(f, v, list, "BIRTHDAY",      sqlSafe(bio.birthdate)       );
    sqlAppend(f, v, list, "CITY",          sqlSafe(bio.city)            );
    sqlAppend(f, v, list, "COUNTRY",       sqlSafe(bio.nationality)     );
    sqlAppend(f, v, list, "ETHNICITY",     sqlSafe(bio.ethnicity)       );
    sqlAppend(f, v, list, "HEIGHT",        sqlSafe(bio.height.toString()));
    sqlAppend(f, v, list, "WEIGHT",        sqlSafe(bio.weight)          );
    sqlAppend(f, v, list, "MEASUREMENTS",  sqlSafe(bio.measurements)    );
    sqlAppend(f, v, list, "HAIR",          sqlSafe(bio.hair)            );
    sqlAppend(f, v, list, "EYES",          sqlSafe(bio.eyes)            );
    sqlAppend(f, v, list, "TATTOOS",       sqlSafe(bio.tattoos)         );
    sqlAppend(f, v, list, "PIERCINGS",     sqlSafe(bio.piercings)       );
    sqlAppend(f, v, list, "PHOTO",         sqlSafe(headshot)            );
    query = QString("%1) VALUES (%2);").arg(f).arg(v);
    return (list.size() > 0);
}

bool Actor::sqlUpdate(QString &query, QStringList &list){
    bool success = false;
    QString f("UPDATE actors SET ");

    list.clear();
    sqlAppend(f, list, "ALIASES",       sqlSafe(bio.aliases)        );
    sqlAppend(f, list, "BIRTHDAY",      sqlSafe(bio.birthdate)      );
    sqlAppend(f, list, "CITY",          sqlSafe(bio.city)           );
    sqlAppend(f, list, "COUNTRY",       sqlSafe(bio.nationality)    );
    sqlAppend(f, list, "ETHNICITY",     sqlSafe(bio.ethnicity)      );
    sqlAppend(f, list, "HEIGHT",        sqlSafe(bio.height.toString()));
    sqlAppend(f, list, "WEIGHT",        sqlSafe(bio.weight)         );
    sqlAppend(f, list, "MEASUREMENTS",  sqlSafe(bio.measurements)   );
    sqlAppend(f, list, "HAIR",          sqlSafe(bio.hair)           );
    sqlAppend(f, list, "EYES",          sqlSafe(bio.eyes)           );
    sqlAppend(f, list, "TATTOOS",       sqlSafe(bio.tattoos)        );
    sqlAppend(f, list, "PIERCINGS",     sqlSafe(bio.piercings)      );
    sqlAppend(f, list, "PHOTO",         sqlSafe(headshot)           );
    if (list.size() > 0){
        f.append(" WHERE NAME = ?;");
        list << sqlSafe(name);
        query = f;
        success = true;
    }
    return success;
}
