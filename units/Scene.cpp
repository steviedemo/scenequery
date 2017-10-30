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
        int age = r.value(QString("age%1").arg(i)).toString();
        ages.push_back(age);
    }
}

Scene::~Scene(){}

QString Scene::sqlUpdate(void){
    QString f("UPDATE scenes SET ");
    bool prev = false;
    sqlAppend(f, "TITLE", sqlSafe(title), prev);
    sqlAppend(f, "COMPANY", sqlSafe(company), prev);
    sqlAppend(f, "SERIES", sqlSafe(series), prev);
    sqlAppend(f, "RATING", sqlSafe(rating), prev);
    sqlAppend(f, "SCENE_NO", sqlSafe(sceneNumber), prev);
    sqlAppend(f, "SIZE", sqlSafe(size), prev);
    sqlAppend(f, "LENGTH", sqlSafe(length), prev);
    sqlAppend(f, "WIDTH", sqlSafe(width), prev);
    sqlAppend(f, "HEIGHT", sqlSafe(height), prev);
    sqlAppend(f, "TAGS", sqlSafe(listToString(tags)), prev);
    sqlAppend(f, "ADDED", sqlSafe(added), prev);
    sqlAppend(f, "CREATED", sqlSafe(released), prev);
    sqlAppend(f, "ACCESSED", sqlSafe(opened), prev);
    sqlAppend(f, "ACTOR1", sqlSafe(actors.at(0)), prev);
    sqlAppend(f, "ACTOR2", sqlSafe(actors.at(1)), prev);
    sqlAppend(f, "ACTOR3", sqlSafe(actors.at(2)), prev);
    sqlAppend(f, "ACTOR4", sqlSafe(actors.at(3)), prev);
    sqlAppend(f, "AGE1", sqlSafe(ages.at(0)), prev);
    sqlAppend(f, "AGE2", sqlSafe(ages.at(1)), prev);
    sqlAppend(f, "AGE3", sqlSafe(ages.at(2)), prev);
    sqlAppend(f, "AGE4", sqlSafe(ages.at(3)), prev);
    sqlAppend(f, "URL", sqlSafe(url), prev);
    if (prev)
        f.append(QString(" WHERE FILENAME = %1;").arg(file.name()));
    else
        f = "";
    return f;
}

bool Scene::inDatabase(void){
    SQL sql;
    QStringList args;
    QString queryString = QString("SELECT * FROM %1 where ACTOR1 = ? AND TITLE = ? AND SIZE = ? AND HEIGHT = ? AND LENGTH = ?").arg(SCENE_DB);
    args << this->actors.at(0) << this->title << this->size << this->height << this->length;
    bool ok = false;
    bool exists = false;
    QSqlQuery *q = sql.assembleQuery(queryString, args, ok);
    if (ok){
        exists = sql.hasMatch(q);
    }
    return exists;
}

QString Scene::sqlInsert(void){
    QString fields("INSERT INTO SCENES (FILENAME,FILEPATH");
    QString values = QString(") VALUES (%1,%2").arg(sqlSafe(file.name())).arg(sqlSafe(file.path()));
    sqlAppend(fields, values, "TITLE",  sqlSafe(title));
    sqlAppend(fields, values, "COMPANY",sqlSafe(company));
    sqlAppend(fields, values, "SERIES", sqlSafe(series));
    sqlAppend(fields, values, "RATING", sqlSafe(rating));
    sqlAppend(fields, values, "LENGTH", sqlSafe(length));
    sqlAppend(fields, values, "SIZE",   sqlSafe(size));
    sqlAppend(fields, values, "WIDTH",  sqlSafe(width));
    sqlAppend(fields, values, "HEIGHT", sqlSafe(height));
    sqlAppend(fields, values, "ADDED",  sqlSafe(added));
    sqlAppend(fields, values, "CREATED",sqlSafe(released));
    sqlAppend(fields, values, "ACCESSED",sqlSafe(opened));
    sqlAppend(fields, values, "TAGS",   sqlSafe(listToString(tags)));
    sqlAppend(fields, values, "ACTOR1", sqlSafe(actors.at(0)));
    sqlAppend(fields, values, "ACTOR2", sqlSafe(actors.at(1)));
    sqlAppend(fields, values, "ACTOR3", sqlSafe(actors.at(2)));
    sqlAppend(fields, values, "ACTOR4", sqlSafe(actors.at(3)));
    sqlAppend(fields, values, "AGE1",   sqlSafe(ages.at(0)));
    sqlAppend(fields, values, "AGE2",   sqlSafe(ages.at(1)));
    sqlAppend(fields, values, "AGE3",   sqlSafe(ages.at(2)));
    sqlAppend(fields, values, "AGE4",   sqlSafe(ages.at(3)));
    sqlAppend(fields, values, "URL",    sqlSafe(url));
    return QString("%1%2);").arg(fields).arg(values);
}

bool sqlInsertScene(const Scene *s, QString &query, QStringList &list){
    list.clear();
    query.clear();
    QString f("INSERT INTO scenes (FILENAME,FILEPATH"), v("?,?");
    list << sqlSafe(s->file.name) << sqlSafe(s->file.path);
    sqlAppend(f, v, list, "TITLE",       sqlSafe(s->title));
    sqlAppend(f, v, list, "COMPANY",     sqlSafe(s->company));
    sqlAppend(f, v, list, "SERIES",      sqlSafe(s->series));
    sqlAppend(f, v, list, "RATING",      sqlSafe(s->rating));
    sqlAppend(f, v, list, "SCENE_NO",    sqlSafe(s->sceneNumber));
    sqlAppend(f, v, list, "SIZE",        sqlSafe(s->size));
    sqlAppend(f, v, list, "LENGTH",      sqlSafe(s->length));
    sqlAppend(f, v, list, "WIDTH",       sqlSafe(s->width));
    sqlAppend(f, v, list, "HEIGHT",      sqlSafe(s->height));
    sqlAppend(f, v, list, "TAGS",        sqlSafe(listToString(s->tags)));
    sqlAppend(f, v, list, "ADDED",       sqlSafe(s->added));
    sqlAppend(f, v, list, "CREATED",     sqlSafe(s->released));
    sqlAppend(f, v, list, "ACCESSED",    sqlSafe(s->opened));
    sqlAppend(f, v, list, "ACTOR1",      sqlSafe(s->actors.at(0)));
    sqlAppend(f, v, list, "ACTOR2",      sqlSafe(s->actors.at(1)));
    sqlAppend(f, v, list, "ACTOR3",      sqlSafe(s->actors.at(2)));
    sqlAppend(f, v, list, "ACTOR4",      sqlSafe(s->actors.at(3)));
    sqlAppend(f, v, list, "AGE1",        sqlSafe(s->ages.at(0)));
    sqlAppend(f, v, list, "AGE2",        sqlSafe(s->ages.at(1)));
    sqlAppend(f, v, list, "AGE3",        sqlSafe(s->ages.at(2)));
    sqlAppend(f, v, list, "AGE4",        sqlSafe(s->ages.at(3)));
    sqlAppend(f, v, list, "URL",         sqlSafe(s->url));
    query = QString("%1) VALUES (%2").arg(f).arg(v);
    query.append(");");
    return (list.size() > 0);
}

bool sqlUpdateScene(const Scene *s, QString &query, QStringList &list){
    QString f("UPDATE scenes SET ");
    list.clear();
    query.clear();

    sqlAppend(f, list, "TITLE",       sqlSafe(s->title));
    sqlAppend(f, list, "COMPANY",     sqlSafe(s->company));
    sqlAppend(f, list, "SERIES",      sqlSafe(s->series));
    sqlAppend(f, list, "RATING",      sqlSafe(s->rating));
    sqlAppend(f, list, "SCENE_NO",    sqlSafe(s->sceneNumber));
    sqlAppend(f, list, "SIZE",        sqlSafe(s->size));
    sqlAppend(f, list, "LENGTH",      sqlSafe(s->length));
    sqlAppend(f, list, "WIDTH",       sqlSafe(s->width));
    sqlAppend(f, list, "HEIGHT",      sqlSafe(s->height));
    sqlAppend(f, list, "TAGS",        sqlSafe(listToString(s->tags)));
    sqlAppend(f, list, "ADDED",       sqlSafe(s->added));
    sqlAppend(f, list, "CREATED",     sqlSafe(s->released));
    sqlAppend(f, list, "ACCESSED",    sqlSafe(s->opened));
    sqlAppend(f, list, "ACTOR1",      sqlSafe(s->actors.at(0)));
    sqlAppend(f, list, "ACTOR2",      sqlSafe(s->actors.at(1)));
    sqlAppend(f, list, "ACTOR3",      sqlSafe(s->actors.at(2)));
    sqlAppend(f, list, "ACTOR4",      sqlSafe(s->actors.at(3)));
    sqlAppend(f, list, "AGE1",        sqlSafe(s->ages.at(0)));
    sqlAppend(f, list, "AGE2",        sqlSafe(s->ages.at(1)));
    sqlAppend(f, list, "AGE3",        sqlSafe(s->ages.at(2)));
    sqlAppend(f, list, "AGE4",        sqlSafe(s->ages.at(3)));
    sqlAppend(f, list, "URL",         sqlSafe(s->url));
    if (list.size() > 0){
        f.append(" WHERE FILENAME = ?;");
        list << sqlSafe(s->file.name);
        query = f;
    }
    return (list.size() > 0);
}

bool Scene::sqlInsert(QString &query, QStringList &list){
    return sqlInsertScene(this, query, list);
}
bool Scene::sqlUpdate(QString &query, QStringList &list){
    return sqlUpdateScene(this, query, list);
}
