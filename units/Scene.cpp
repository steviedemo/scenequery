#include "Scene.h"
#include "Rating.h"
#include "genericfunctions.h"
#include "sceneParser.h"
#include "sql.h"
#include "query.h"
#include <QSqlQuery>
#include <QVariant>
Scene::Scene(){
    actors  = {};
    ages    = {};
}
Scene::Scene(sceneParser s){
    if (!s.isEmpty()){
        if (!s.isParsed()){
            s.parse();
        }
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
    } else {
        actors  = {};
        ages    = {};
    }
}

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

Query Scene::toQuery(){
    Query q();
    q.add("TITLE", title);
    q.add("COMPANY", company);
    q.add("SERIES", series);
    q.add("RATING", rating);
    q.add("SCENE_NO", sceneNumber);
    q.add("SIZE",   size);
    q.add("LENGTH", length);
    q.add("WIDTH", width);
    q.add("HEIGHT", height);
    q.add("TAGS",  sqlSafe(listToString(this->tags)));
    q.add("ADDED", added);
    q.add("CREATED", released);
    q.add("ACCESSED", opened);
    q.add("ACTOR1", actors.at(0));
    q.add("ACTOR2", actors.at(1));
    q.add("ACTOR3", actors.at(2));
    q.add("ACTOR4", actors.at(3));
    q.add("AGE1", ages.at(0));
    q.add("AGE2", ages.at(1));
    q.add("AGE3", ages.at(2));
    q.add("AGE4", ages.at(3));
    q.add("URL", url);
    return q;
}



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
