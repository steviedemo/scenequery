#include "Scene.h"
#include "Rating.h"
#include "genericfunctions.h"
#include "sceneParser.h"
#include "sql.h"
#include <QVariant>
Scene::Scene(){}
Scene::Scene(FilePath file){
    this->file      = file;
    sceneParser p;
    p.parse(file);
    this->actors    = p.getActors();
    this->title     = p.getTitle();
    this->company   = p.getCompany();
    this->series    = p.getSeries();
    this->rating    = p.getRating();
    this->tags      = p.getTags();
    this->width     = p.getWidth();
    this->height    = p.getHeight();
    this->size      = p.getSize();
    this->length    = p.getLength();
    this->added     = p.getAdded();
    this->opened    = p.getAccessed();
    this->released  = p.getReleased();
    this->ages      = QVector<int>(actors.size(), 0);
}

Scene::Scene(QSqlRecord r){
    this->file      = FilePath(r.value("filepath").toString(), r.value("filename").toString());
    this->title     = r.value("title").toString();
    this->company   = r.value("company").toString();
    this->series    = r.value("series").toString();
    this->rating    = r.value("rating").toString();
    this->tags      = r.value("tags").toString().split(',');
    this->width     = r.value("width").toInt();
    this->height    = r.value("height").toInt();
    this->size      = r.value("size").toInt();
    this->length    = r.value("length").toDouble();
    this->added     = QDate::fromString(r.value("added").toString(),    "yyyy.MM.dd");
    this->opened    = QDate::fromString(r.value("accessed").toString(), "yyyy.MM.dd");
    this->released  = QDate::fromString(r.value("created").toString(),  "yyyy.MM.dd");
    this->url       = r.value("url").toString();
    for (int i = 1; i <= 4; ++i){
        QString name = r.value(QString("actor%1").arg(i)).toString();
        if (!name.isEmpty()){
            this->actors.push_back(name);
        }
        int age = r.value(QString("age%1").arg(i)).toInt();
        ages.push_back(age);
    }
}

Scene::~Scene(){}

//QString Scene::sqlUpdate(void){
//    QString f("UPDATE scenes SET ");
//    bool prev = false;
//    SQL::sqlAppend(f, "TITLE", sqlSafe(title), prev);
//    SQL::sqlAppend(f, "COMPANY", sqlSafe(company), prev);
//    SQL::sqlAppend(f, "SERIES", sqlSafe(series), prev);
//    SQL::sqlAppend(f, "RATING", sqlSafe(rating.toString()), prev);
//    SQL::sqlAppend(f, "SCENE_NO", sqlSafe(sceneNumber), prev);
//    SQL::sqlAppend(f, "SIZE", sqlSafe(size), prev);
//    SQL::sqlAppend(f, "LENGTH", sqlSafe(length), prev);
//    SQL::sqlAppend(f, "WIDTH", sqlSafe(width), prev);
//    SQL::sqlAppend(f, "HEIGHT", sqlSafe(height), prev);
//    SQL::sqlAppend(f, "TAGS", sqlSafe(listToString(tags)), prev);
//    SQL::sqlAppend(f, "ADDED", sqlSafe(added), prev);
//    SQL::sqlAppend(f, "CREATED", sqlSafe(released), prev);
//    SQL::sqlAppend(f, "ACCESSED", sqlSafe(opened), prev);
//    SQL::sqlAppend(f, "ACTOR1", sqlSafe(actors.at(0)), prev);
//    SQL::sqlAppend(f, "ACTOR2", sqlSafe(actors.at(1)), prev);
//    SQL::sqlAppend(f, "ACTOR3", sqlSafe(actors.at(2)), prev);
//    SQL::sqlAppend(f, "ACTOR4", sqlSafe(actors.at(3)), prev);
//    SQL::sqlAppend(f, "AGE1", sqlSafe(ages.at(0)), prev);
//    SQL::sqlAppend(f, "AGE2", sqlSafe(ages.at(1)), prev);
//    SQL::sqlAppend(f, "AGE3", sqlSafe(ages.at(2)), prev);
//    SQL::sqlAppend(f, "AGE4", sqlSafe(ages.at(3)), prev);
//    SQL::sqlAppend(f, "URL", sqlSafe(url), prev);
//    if (prev)
//        f.append(QString(" WHERE FILENAME = %1;").arg(file.getName()));
//    else
//        f = "";
//    return f;
//}

bool Scene::inDatabase(void){
    SQL sql;
    QStringList args;
    QString queryString = QString("SELECT * FROM %1 where ACTOR1 = ? AND TITLE = ? AND SIZE = ? AND HEIGHT = ? AND LENGTH = ?").arg(SCENE_DB);
    args << this->actors.at(0) << this->title << QString("%1").arg(this->size) << QString("%1").arg(this->height) << QString("%1").arg(this->length);
    bool ok = false;
    bool exists = false;
    QSharedPointer<QSqlQuery>q = sql.assembleQuery(queryString, args, ok);
    if (ok){
        exists = sql.hasMatch(q.data());
    }
    return exists;
}

//QString Scene::sqlInsert(void){
//    QString fields("INSERT INTO SCENES (FILENAME,FILEPATH");
//    QString values = QString(") VALUES (%1,%2").arg(sqlSafe(file.getName())).arg(sqlSafe(file.getPath()));
//    SQL::sqlAppend(fields, values, "TITLE",  sqlSafe(title));
//    SQL::sqlAppend(fields, values, "COMPANY",sqlSafe(company));
//    SQL::sqlAppend(fields, values, "SERIES", sqlSafe(series));
//    SQL::sqlAppend(fields, values, "RATING", sqlSafe(rating));
//    SQL::sqlAppend(fields, values, "LENGTH", sqlSafe(length));
//    SQL::sqlAppend(fields, values, "SIZE",   sqlSafe(size));
//    SQL::sqlAppend(fields, values, "WIDTH",  sqlSafe(width));
//    SQL::sqlAppend(fields, values, "HEIGHT", sqlSafe(height));
//    SQL::sqlAppend(fields, values, "ADDED",  sqlSafe(added));
//    SQL::sqlAppend(fields, values, "CREATED",sqlSafe(released));
//    SQL::sqlAppend(fields, values, "ACCESSED",sqlSafe(opened));
//    SQL::sqlAppend(fields, values, "TAGS",   sqlSafe(listToString(tags)));
//    SQL::sqlAppend(fields, values, "ACTOR1", sqlSafe(actors.at(0)));
//    SQL::sqlAppend(fields, values, "ACTOR2", sqlSafe(actors.at(1)));
//    SQL::sqlAppend(fields, values, "ACTOR3", sqlSafe(actors.at(2)));
//    SQL::sqlAppend(fields, values, "ACTOR4", sqlSafe(actors.at(3)));
//    SQL::sqlAppend(fields, values, "AGE1",   sqlSafe(ages.at(0)));
//    SQL::sqlAppend(fields, values, "AGE2",   sqlSafe(ages.at(1)));
//    SQL::sqlAppend(fields, values, "AGE3",   sqlSafe(ages.at(2)));
//    SQL::sqlAppend(fields, values, "AGE4",   sqlSafe(ages.at(3)));
//    SQL::sqlAppend(fields, values, "URL",    sqlSafe(url));
//    return QString("%1%2);").arg(fields).arg(values);
//}

bool Scene::sqlInsert(QString &query, QStringList &list) const{
    list.clear();
    query.clear();
    QString f("INSERT INTO scenes (FILENAME,FILEPATH"), v("?,?");
    list << SQL::sqlSafe(file.getName());
    list << SQL::sqlSafe(file.getPath());
    SQL::sqlAppend(f, v, list, "TITLE",       SQL::sqlSafe(this->title));
    SQL::sqlAppend(f, v, list, "COMPANY",     SQL::sqlSafe(this->company));
    SQL::sqlAppend(f, v, list, "SERIES",      SQL::sqlSafe(this->series));
    SQL::sqlAppend(f, v, list, "RATING",      SQL::sqlSafe(this->rating.toString()));
    SQL::sqlAppend(f, v, list, "SCENE_NO",    SQL::sqlSafe(this->sceneNumber));
    SQL::sqlAppend(f, v, list, "SIZE",        SQL::sqlSafe(this->size));
    SQL::sqlAppend(f, v, list, "LENGTH",      SQL::sqlSafe(this->length));
    SQL::sqlAppend(f, v, list, "WIDTH",       SQL::sqlSafe(this->width));
    SQL::sqlAppend(f, v, list, "HEIGHT",      SQL::sqlSafe(this->height));
    SQL::sqlAppend(f, v, list, "TAGS",        SQL::sqlSafe(listToString(this->tags)));
    SQL::sqlAppend(f, v, list, "ADDED",       SQL::sqlSafe(this->added));
    SQL::sqlAppend(f, v, list, "CREATED",     SQL::sqlSafe(this->released));
    SQL::sqlAppend(f, v, list, "ACCESSED",    SQL::sqlSafe(this->opened));
    SQL::sqlAppend(f, v, list, "ACTOR1",      SQL::sqlSafe(this->actors.at(0)));
    SQL::sqlAppend(f, v, list, "ACTOR2",      SQL::sqlSafe(this->actors.at(1)));
    SQL::sqlAppend(f, v, list, "ACTOR3",      SQL::sqlSafe(this->actors.at(2)));
    SQL::sqlAppend(f, v, list, "ACTOR4",      SQL::sqlSafe(this->actors.at(3)));
    SQL::sqlAppend(f, v, list, "AGE1",        SQL::sqlSafe(this->ages.at(0)));
    SQL::sqlAppend(f, v, list, "AGE2",        SQL::sqlSafe(this->ages.at(1)));
    SQL::sqlAppend(f, v, list, "AGE3",        SQL::sqlSafe(this->ages.at(2)));
    SQL::sqlAppend(f, v, list, "AGE4",        SQL::sqlSafe(this->ages.at(3)));
    SQL::sqlAppend(f, v, list, "URL",         SQL::sqlSafe(this->url));
    query = QString("%1) VALUES (%2").arg(f).arg(v);
    query.append(");");
    return (list.size() > 0);
}

bool Scene::sqlUpdate(QString &query, QStringList &list) const{
    QString f("UPDATE scenes SET ");
    list.clear();
    query.clear();

    SQL::sqlAppend(f, list, "TITLE",       SQL::sqlSafe(this->title));
    SQL::sqlAppend(f, list, "COMPANY",     SQL::sqlSafe(this->company));
    SQL::sqlAppend(f, list, "SERIES",      SQL::sqlSafe(this->series));
    SQL::sqlAppend(f, list, "RATING",      SQL::sqlSafe(this->rating.toString()));
    SQL::sqlAppend(f, list, "SCENE_NO",    SQL::sqlSafe(this->sceneNumber));
    SQL::sqlAppend(f, list, "SIZE",        SQL::sqlSafe(this->size));
    SQL::sqlAppend(f, list, "LENGTH",      SQL::sqlSafe(this->length));
    SQL::sqlAppend(f, list, "WIDTH",       SQL::sqlSafe(this->width));
    SQL::sqlAppend(f, list, "HEIGHT",      SQL::sqlSafe(this->height));
    SQL::sqlAppend(f, list, "TAGS",        SQL::sqlSafe(listToString(this->tags)));
    SQL::sqlAppend(f, list, "ADDED",       SQL::sqlSafe(this->added));
    SQL::sqlAppend(f, list, "CREATED",     SQL::sqlSafe(this->released));
    SQL::sqlAppend(f, list, "ACCESSED",    SQL::sqlSafe(this->opened));
    SQL::sqlAppend(f, list, "ACTOR1",      SQL::sqlSafe(this->actors.at(0)));
    SQL::sqlAppend(f, list, "ACTOR2",      SQL::sqlSafe(this->actors.at(1)));
    SQL::sqlAppend(f, list, "ACTOR3",      SQL::sqlSafe(this->actors.at(2)));
    SQL::sqlAppend(f, list, "ACTOR4",      SQL::sqlSafe(this->actors.at(3)));
    SQL::sqlAppend(f, list, "AGE1",        SQL::sqlSafe(this->ages.at(0)));
    SQL::sqlAppend(f, list, "AGE2",        SQL::sqlSafe(this->ages.at(1)));
    SQL::sqlAppend(f, list, "AGE3",        SQL::sqlSafe(this->ages.at(2)));
    SQL::sqlAppend(f, list, "AGE4",        SQL::sqlSafe(this->ages.at(3)));
    SQL::sqlAppend(f, list, "URL",         SQL::sqlSafe(this->url));
    if (list.size() > 0){
        f.append(" WHERE FILENAME = ?;");
        list << SQL::sqlSafe(this->file.getName());
        query = f;
    }
    return (list.size() > 0);
}
