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
//    SQL::sqlAppend(f, v, "ALIASES", sqlSafe(bio.aliases));
//    SQL::sqlAppend(f, v, "BIRTHDAY", sqlSafe(bio.birthdate));
//    SQL::sqlAppend(f, v, "CITY", sqlSafe(bio.city));
//    SQL::sqlAppend(f, v, "COUNTRY", sqlSafe(bio.nationality));
//    SQL::sqlAppend(f, v, "ETHNICITY", sqlSafe(bio.ethnicity));
//    SQL::sqlAppend(f, v, "HEIGHT", sqlSafe(bio.height.toString()));
//    SQL::sqlAppend(f, v, "WEIGHT", sqlSafe(bio.weight));
//    SQL::sqlAppend(f, v, "MEASUREMENTS", sqlSafe(bio.measurements));
//    SQL::sqlAppend(f, v, "HAIR", sqlSafe(bio.hair));
//    SQL::sqlAppend(f, v, "EYES", sqlSafe(bio.eyes));
//    SQL::sqlAppend(f, v, "TATTOOS", sqlSafe(bio.tattoos));
//    SQL::sqlAppend(f, v, "PIERCINGS", sqlSafe(bio.piercings));
//    SQL::sqlAppend(f, v, "PHOTO", sqlSafe(headshot));
//}

//QString Actor::sqlUpdate(void){
//    QString f("UPDATE actors SET ");
//    bool p = false;
//    SQL::sqlAppend(f, "ALIASES", sqlSafe(bio.aliases), p);
//    SQL::sqlAppend(f, "BIRTHDAY", sqlSafe(bio.birthdate), p);
//    SQL::sqlAppend(f, "CITY", sqlSafe(bio.city), p);
//    SQL::sqlAppend(f, "COUNTRY", sqlSafe(bio.nationality), p);
//    SQL::sqlAppend(f, "ETHNICITY", sqlSafe(bio.ethnicity), p);
//    SQL::sqlAppend(f, "HEIGHT", sqlSafe(bio.height.toString()), p);
//    SQL::sqlAppend(f, "WEIGHT", sqlSafe(bio.weight), p);
//    SQL::sqlAppend(f, "MEASUREMENTS", sqlSafe(bio.measurements), p);
//    SQL::sqlAppend(f, "HAIR", sqlSafe(bio.hair), p);
//    SQL::sqlAppend(f, "EYES", sqlSafe(bio.eyes), p);
//    SQL::sqlAppend(f, "TATTOOS", sqlSafe(bio.tattoos), p);
//    SQL::sqlAppend(f, "PIERCINGS", sqlSafe(bio.piercings), p);
//    SQL::sqlAppend(f, "PHOTO", sqlSafe(headshot), p);
//    if (p){
//        f.append(QString(" WHERE NAME = %1").arg(name));
//    }
//    return f;
//}

bool Actor::sqlInsert(QString &query, QStringList &list){
    list.clear();
    QString f ("INSERT INTO ACTORS (NAME"), v("?");
    list << SQL::sqlSafe(name);
    SQL::sqlAppend(f, v, list, "ALIASES",       SQL::sqlSafe(bio.aliases)         );
    SQL::sqlAppend(f, v, list, "BIRTHDAY",      SQL::sqlSafe(bio.birthdate)       );
    SQL::sqlAppend(f, v, list, "CITY",          SQL::sqlSafe(bio.city)            );
    SQL::sqlAppend(f, v, list, "COUNTRY",       SQL::sqlSafe(bio.nationality)     );
    SQL::sqlAppend(f, v, list, "ETHNICITY",     SQL::sqlSafe(bio.ethnicity)       );
    SQL::sqlAppend(f, v, list, "HEIGHT",        SQL::sqlSafe(bio.height.toString()));
    SQL::sqlAppend(f, v, list, "WEIGHT",        SQL::sqlSafe(bio.weight)          );
    SQL::sqlAppend(f, v, list, "MEASUREMENTS",  SQL::sqlSafe(bio.measurements)    );
    SQL::sqlAppend(f, v, list, "HAIR",          SQL::sqlSafe(bio.hair)            );
    SQL::sqlAppend(f, v, list, "EYES",          SQL::sqlSafe(bio.eyes)            );
    SQL::sqlAppend(f, v, list, "TATTOOS",       SQL::sqlSafe(bio.tattoos)         );
    SQL::sqlAppend(f, v, list, "PIERCINGS",     SQL::sqlSafe(bio.piercings)       );
    SQL::sqlAppend(f, v, list, "PHOTO",         SQL::sqlSafe(headshot)            );
    query = QString("%1) VALUES (%2);").arg(f).arg(v);
    return (list.size() > 0);
}

bool Actor::sqlUpdate(QString &query, QStringList &list){
    bool success = false;
    QString f("UPDATE actors SET ");

    list.clear();
    SQL::sqlAppend(f, list, "ALIASES",       SQL::sqlSafe(bio.aliases)        );
    SQL::sqlAppend(f, list, "BIRTHDAY",      SQL::sqlSafe(bio.birthdate)      );
    SQL::sqlAppend(f, list, "CITY",          SQL::sqlSafe(bio.city)           );
    SQL::sqlAppend(f, list, "COUNTRY",       SQL::sqlSafe(bio.nationality)    );
    SQL::sqlAppend(f, list, "ETHNICITY",     SQL::sqlSafe(bio.ethnicity)      );
    SQL::sqlAppend(f, list, "HEIGHT",        SQL::sqlSafe(bio.height.toString()));
    SQL::sqlAppend(f, list, "WEIGHT",        SQL::sqlSafe(bio.weight)         );
    SQL::sqlAppend(f, list, "MEASUREMENTS",  SQL::sqlSafe(bio.measurements)   );
    SQL::sqlAppend(f, list, "HAIR",          SQL::sqlSafe(bio.hair)           );
    SQL::sqlAppend(f, list, "EYES",          SQL::sqlSafe(bio.eyes)           );
    SQL::sqlAppend(f, list, "TATTOOS",       SQL::sqlSafe(bio.tattoos)        );
    SQL::sqlAppend(f, list, "PIERCINGS",     SQL::sqlSafe(bio.piercings)      );
    SQL::sqlAppend(f, list, "PHOTO",         SQL::sqlSafe(headshot)           );
    if (list.size() > 0){
        f.append(" WHERE NAME = ?;");
        list << SQL::sqlSafe(name);
        query = f;
        success = true;
    }
    return success;
}
