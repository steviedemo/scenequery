#include "Scene.h"
#include "Rating.h"
#include "filenames.h"
#include "genericfunctions.h"
#include "SceneParser.h"
#include "SQL.h"
#include "Query.h"
////#include <QSqlQuery>
#include <QVariant>
#include <QRegularExpression>
#include <string>
#ifdef DEBUG
#define trace(); qDebug("%s::%s::%d", __FILE__, __FUNCTION__, __LINE__);
#else
#define trace();
#endif
using namespace std;
bool operator==(const ScenePtr &s1, const ScenePtr &s2){
    return (s1->equals(s2));
}
bool operator==(const Scene &s1, const Scene &s2){
    return s1.equals(s2);
}

Scene::Scene(const QString absolutePath): Entry(){
    this->clear();
    if (valid(absolutePath)){
        this->fromParser(SceneParser(absolutePath));
    }
}

void Scene::clear(){
    this->ID            = -1;
    this->height        = -1;
    this->width         = -1;
    this->sceneNumber   = -1;
    this->size          = -1;
    this->added         = QDate();
    this->opened        = QDate();
    this->released      = QDate();
    this->length        = QTime(0,0,0,0);
    this->title         = QString("");
    this->company       = QString("");
    this->series        = QString("");
    this->url           = QString("");
    this->dateString    = QString("");
    this->filename      = QString("");
    this->filepath      = QString("");
    this->displayBuilt  = false;
    this->actors        = QVector<QString>(0);
    this->tags          = QVector<QString>(0);
    this->ages          = QVector<int>(0);
    this->row           = QList<QStandardItem *>();
    this->rating = Rating("R");
}

void Scene::reparse(){
    QString fullpath = getFullpath();
    SceneParser p(fullpath);
    p.parse();
    fromParser(p);
}

void Scene::fromParser(SceneParser p){
    if (!p.isEmpty()){
        if (!p.isParsed()){
            p.parse();
        }
        actors = QVector<QString>(0);
        tags = QVector<QString>(0);
        this->filepath = p.getFilepath();
        this->filename = p.getFilename();
        //qDebug("Making Scene with name: '%s', and path: '%s'", qPrintable(filename), qPrintable(filepath));
        QVector<QString> list = p.getActors();
        if (list.size() > 0){
            foreach(QString s, list){
                actors.push_back(s);
            }
        }
        list = {};
        list = p.getTags();
        if (list.size() > 0){
            foreach(QString s, list){
                tags.push_back(s);
            }
        }
        this->title     = p.getTitle();
        this->company   = p.getCompany();
        this->series    = p.getSeries();
        this->rating    = p.getRating();
        this->width     = p.getWidth();
        this->height    = p.getHeight();
        this->size      = p.getSize();
        this->length    = p.getLength();
        this->added     = p.getAdded();
        this->opened    = p.getAccessed();
        this->released  = p.getReleased();
        this->ages      = QVector<int>(actors.size(), 0);
    } else {
        ages = {};
    }
    displayBuilt = false;
}


Scene::Scene(QSqlRecord r){
    this->ID        = r.value("id").toLongLong();
    this->filename  = r.value("filename").toString();
    this->filepath  = r.value("filepath").toString();
    this->title     = r.value("title").toString();
    this->company   = r.value("company").toString();
    this->series    = r.value("series").toString();
    this->rating.fromString(r.value("rating").toString());
    this->tags      = r.value("tags").toString().split(',').toVector();
    this->width     = r.value("width").toInt();
    this->height    = r.value("height").toInt();
    this->size      = r.value("size").toInt();
    double minutes  = r.value("length").toDouble();
    QTime t(0,0,0);
    this->length    = t.addSecs((int)(minutes*60));
    this->added     = QDate::fromString(r.value("added").toString(),    "yyyy-MM-dd");
    this->opened    = QDate::fromString(r.value("accessed").toString(), "yyyy-MM-dd");
    this->released  = QDate::fromString(r.value("created").toString(),  "yyyy-MM-dd");
    this->url       = r.value("url").toString();
    for (int i = 1; i <= 4; ++i){
        QString name = r.value(QString("actor%1").arg(i)).toString();
        if (!name.isEmpty()){
            this->actors.push_back(name);
        }
        int age = r.value(QString("age%1").arg(i)).toInt();
        ages.push_back(age);
    }
    displayBuilt = false;
}


void Scene::setLength(const QTime &t){
    if (!t.isNull() && t.isValid()){
        this->length = QTime(t);
    }
}
void Scene::setCompany(const QString &c){
    if (!c.isNull() && !c.isEmpty() && !c.isDetached()){
        this->company = QString(c);
    } else {
        qWarning("Error: Attempting to set Company to an empty or invalid String");
    }
}
void Scene::setSeries(const QString &s){
    if (!s.isNull() && !s.isEmpty()){
        this->series = QString(s);
    }
}
void Scene::setReleased(const QDate &d){
    if (!d.isNull() && d.isValid()){
        this->released = QDate(d);
    } else {
        qWarning("Error: Attempting to set Release Date to an empty or invalid Date");
    }
}
void Scene::setTitle(const QString &t){
    if (!t.isNull() && !t.isEmpty() && !t.isDetached()){
        this->title = QString(t);
    } else {
        qWarning("Error: Attempting to set title to empty or invalid string");
    }
}
void Scene::addActor(QString a){
    if (!actors.isEmpty()){
        QVector<int> emptyIdxs = {};
        for (int i = 0; i < actors.size(); ++i){
            if (actors.at(i) == ""){
                emptyIdxs.push_back(i);
            }
        }
        foreach(int idx, emptyIdxs){
            actors.removeAt(idx);
        }
    }
    if (!a.isEmpty() && !this->actors.contains(a)){
        actors << a;
    }
}
void Scene::removeActor(QString a){
    if (!a.isEmpty() && a.contains(a)){
        QVector<int> idxs = {};
        for(int i = 0; i < actors.size(); ++i){
            if (actors.at(i) == a){
                idxs.push_back(i);
            }
        }
        foreach(int idx, idxs){
            actors.removeAt(idx);
        }
    }
}

void Scene::renameActor(QString oldName, QString newName){
    if (!oldName.isEmpty() && !newName.isEmpty() && actors.contains(oldName)){
        for(int i = 0; i < actors.size(); ++i){
            if (actors.at(i) == oldName){
                actors[i] = newName;
            }
        }
    }
}



void Scene::fromRecord(pqxx::result::const_iterator entry){
    try{
        QRegularExpression dateRegex("[0-9]{4}-[0-9]{2}-[0-9]{2}");
        QRegularExpressionMatch dateMatch;
        trace();
        if (!entry.at("id").is_null())     {
            this->ID = entry["id"].as<int>();
        }
        trace();

        if (!entry.at("filepath").is_null()){
            filepath = QString::fromStdString(entry["filepath"].as<string>());
        }
        if (!entry.at("filename").is_null()){
            filename = QString::fromStdString(entry["filename"].as<string>());
        }
        if (!filename.isEmpty() && !filepath.isEmpty()){
            this->setFile(file);
        }
        if (!entry.at("title").is_null())  {   this->title     = QString::fromStdString(entry["title"].as<std::string>());     }
        if (!entry.at("company").is_null()){   this->company   = QString::fromStdString(entry["company"].as<std::string>());   }
        if (!entry.at("series").is_null()) {   this->series    = QString::fromStdString(entry["series"].as<std::string>());    }
        if (!entry.at("rating").is_null()) {   this->rating.fromString(QString::fromStdString(entry["rating"].as<std::string>()));                    }
        entry["scene_no"].to(sceneNumber);
        entry["size"].to(size);
        entry["width"].to(width);
        entry["height"].to(height);
        if (!entry.at("length").is_null()) {
            QString time = QString::fromStdString(entry["length"].as<std::string>());
            QTime temp = QTime::fromString(time, "h:mm:ss");
            this->length = temp;
        }
        if (!entry.at("added").is_null())  {
            QString temp = QString::fromStdString(entry["added"].as<std::string>());
            dateMatch = dateRegex.match(temp);
            if (dateMatch.hasMatch()){
                this->added = QDate::fromString(temp, "yyyy-MM-dd");
            } else { this->added = QDate(); }
        } else { this->added = QDate(); }

        if (!entry.at("created").is_null()){
            QString temp = QString::fromStdString(entry["created"].as<std::string>());
            dateMatch = dateRegex.match(temp);
            if (dateMatch.hasMatch()){
                this->released = QDate::fromString(temp, "yyyy-MM-dd");
                this->dateString = temp.replace('-', '.');
            } else {
                this->released = QDate();
                this->dateString = "";
            }
        } else {
            released = QDate();
            dateString = "";
        }
        if (!entry.at("accessed").is_null()){
            QString temp = QString::fromStdString(entry["accessed"].as<std::string>());
            dateMatch = dateRegex.match(temp);
            if (dateMatch.hasMatch()){
                this->opened = QDate::fromString(temp, "yyyy-MM-dd");
            } else {
                this->opened = QDate();
            }
        } else {
            this->opened = QDate();
        }
        if (!entry.at("tags").is_null()){
            this->tags = QString::fromStdString(entry["tags"].as<std::string>()).split(',', QString::SkipEmptyParts).toVector();
        }
        bool error = false;
        for (int idx = 0; idx < 4 && !error; ++idx){
            try{
                std::string fieldname = qPrintable(QString("actor%1").arg(idx+1));
                if (!entry.at(fieldname).is_null()){
                    actors << QString::fromStdString(entry[fieldname].as<std::string>());
                    std::string fieldage  = qPrintable(QString("age%1").arg(idx+1));
                    if (!entry.at(fieldage).is_null()){
                        ages.push_back(entry[fieldage].as<int>());
                    }
                }
            } catch (std::out_of_range &e){
                qWarning("Error: index %d out of range - %s", idx, e.what());
                error = true;
            }
        }
    }catch(std::exception &e){
        qWarning("Exception Caught while making Scene: %s", e.what());
    } catch (...){
        qWarning("Unknown Exception Caught While Making Scene");
    }
}

bool Scene::equals(const QString &path) const{
    bool same = false;
    if (valid(path)){
        if (valid(filepath)){
            if (valid(filename)){
                QString fullpath = QString("%1/%2").arg(filepath).arg(filename);
                if (fullpath == path){
                    same = true;
                }
            }
        }
    }
    return same;
}
bool Scene::equals(const QPair<QString, QString> &p) const{
    bool same = false;
    if (valid(p.first) && valid(p.second) && valid(filepath) && valid(filename)){
        same = (p.first == filepath) && (p.second == filename);
    }
    return same;
}

QList<QStandardItem *> Scene::getQStandardItem(){
    if (!displayBuilt){
        qDebug("Building new Scene Item for %s", qPrintable(title));
        return this->buildQStandardItem();
    } else {

    }
    row << itemActors << itemTitle << itemCompany << itemQuality \
        << itemSize << itemLength << itemDate << itemRating << itemID;
    return row;
}

QList<QStandardItem *> Scene::buildQStandardItem(){
    QString sizeString("");
    if(size > BYTES_PER_GIGABYTE){
        double gb = (double)(size/BYTES_PER_GIGABYTE);
        sizeString = QString("%1 GB").arg(QString::number(gb, 'f', 2));
    } else if (this->size > BYTES_PER_MEGABYTE){
        sizeString = QString("%1 MB").arg(size/BYTES_PER_MEGABYTE);
    }
    QString path = QString("%1/%2").arg(filepath).arg(filename);
    this->itemSize      = new QStandardItem(sizeString);
    this->itemTitle     = new QStandardItem(title);
    this->itemCompany   = new QStandardItem(company);
    this->itemPath      = new QStandardItem(path);
    this->itemID        = new QStandardItem(QString::number(ID));
    this->itemTags      = new QStandardItem(this->tagString());
    this->itemSeries    = new QStandardItem(series);
    QString date("");
    if (released.isValid()){
        date = released.toString("yyyy/MM/dd");
    }
    this->itemDate = new QStandardItem(date);
    this->itemQuality = new QStandardItem("");
    if (height > 0){
        this->itemQuality->setData(QVariant(height), Qt::DecorationRole);
    }
    this->itemLength = new QStandardItem(length.toString("h:mm:ss"));
    this->itemRating = new QStandardItem(rating.grade());
    QString mainActor(""), featuredActors("");
    if (actors.size() > 0){
        mainActor = actors.at(0).trimmed();
        if (actors.size() > 1){
            QVectorIterator<QString> it(actors);
            it.next();
            while(it.hasNext()){
                QString curr = it.next();
                if (!curr.isEmpty()){
                    if (!featuredActors.isEmpty()){
                        featuredActors.append(", ");
                    }
                    featuredActors += curr;
                }
            }
        }
    }
    this->itemActors = new QStandardItem();
    this->itemActors->setData(QVariant(mainActor), Qt::DisplayRole);
    this->itemFeaturedActors = new QStandardItem();
    this->itemFeaturedActors->setData(QVariant(featuredActors), Qt::DisplayRole);
    row << itemActors << itemTitle << itemCompany << itemQuality \
        << itemFeaturedActors <<  itemSize << itemLength \
        << itemDate << itemRating << itemPath << itemID \
        << itemTags << itemSeries;
    displayBuilt = true;
    return row;
}

void Scene::updateQStandardItem(){
    if (displayBuilt){
        QString mainActor(""), featuredActors("");
        if (actors.size() > 0){
            mainActor = actors.at(0).trimmed();
            if (actors.size() > 1){
                QVectorIterator<QString> it(actors);
                it.next();
                while(it.hasNext()){
                    QString curr = it.next();
                    if (!curr.isEmpty()){
                        if (!featuredActors.isEmpty()){
                            featuredActors.append(", ");
                        }
                        featuredActors += curr;
                    }
                }
            }
        }
        this->itemActors->setData(QVariant(mainActor), Qt::DisplayRole);
        this->itemFeaturedActors->setData(QVariant(featuredActors), Qt::DisplayRole);
        QString date("");
        if (released.isValid()){
            date = released.toString("yyyy/MM/dd");
        }
        this->itemDate->setText(date);
        if (height > 0){
            this->itemQuality->setData(QVariant(height), Qt::DecorationRole);
        } else {
            itemQuality->setText("");
        }
        this->itemTitle->setText(title);
        this->itemLength->setText(length.toString("h:mm:ss"));
        this->itemRating->setText(rating.grade());
        this->itemCompany->setText(company);
    }
}

bool Scene::inDatabase(void){
    bool found = false;
    sqlConnection *sql = new sqlConnection(QString("SELECT * FROM %1 where ACTOR1 = ? AND TITLE = ? AND SIZE = ? AND HEIGHT = ? AND LENGTH = ?").arg(SCENE_DB));
    if (sql->execute()){
        found = sql->foundMatch();
    }
    delete sql;
    return found;
}

QString Scene::tagString() const{
    QString s("");
    QTextStream out(&s);
    QVectorIterator<QString> it(tags);
    while(it.hasNext()){
        out << it.next() << (it.hasNext() ? ", " : "");
    }
    return s;
}

Query Scene::toQuery() const{
    Query q;
    q.setTable("scenes");
    q.addCriteria("ID", QString::number(this->ID));
    q.add("FILEPATH", filepath);
    q.add("FILENAME", filename);
    q.add("TITLE", title);
    q.add("COMPANY", company);
    q.add("SERIES", series);
    q.add("RATING", rating.grade());
    q.add("SCENE_NO", sceneNumber);
    q.add("SIZE",   size);
    q.add("LENGTH", length.toString("h:mm:ss"));
    q.add("WIDTH", width);
    q.add("HEIGHT", height);
    q.add("TAGS",  tagString());
    q.add("TAGS",  listToString(this->tags));
    q.add("ADDED", added);
    q.add("CREATED", released);
    q.add("ACCESSED", opened);
    if (actors.size() > 0)
        q.add("ACTOR1", actors.at(0));
    if (actors.size() > 1)
        q.add("ACTOR2", actors.at(1));
    if (actors.size() > 2)
        q.add("ACTOR3", actors.at(2));
    if (actors.size() > 3)
        q.add("ACTOR4", actors.at(3));
    if (ages.size() > 0)
        q.add("AGE1", ages.at(0));
    if (ages.size() > 1)
        q.add("AGE2", ages.at(1));
    if (ages.size() > 2)
        q.add("AGE3", ages.at(2));
    if (ages.size() > 3)
        q.add("AGE4", ages.at(3));
    if (!url.isEmpty())
        q.add("URL", url);
    return q;
}

int Scene::entrySize(){
    int size = 0;
    size += (isEmpty(file) ? 0 : 1);
    size += (this->title.isEmpty() ? 0 : 1);
    size += (this->company.isEmpty() ? 0 : 1);
    size += (this->series.isEmpty() ? 0 : 1);
    size += (this->rating.isEmpty() ? 0 : 1);
    size += tags.size();
    size += ((width == 0) ? 0 : 1);
    size += ((height == 0) ? 0 : 1);
    size += ((size == 0) ? 0 : 1);
    if (length > QTime(0,0,0)){
        size++;
    }
    size += (this->added.isValid() ? 0 : 1);
    size += (this->opened.isValid() ? 0 : 1);
    size += (this->released.isValid() ? 0 : 1);
    foreach(QString actor, actors){
        if (!actor.isEmpty()){
            size++;
        }
    }
    foreach(int age, ages){
        if (age > 0){
            size++;
        }
    }
    return size;
}



bool Scene::equals(const ScenePtr &other) const {
    bool same = false;
    if (this->title == other->title){
        if (this->size == other->size){
            if (this->length == other->length){
                if (this->height == other->height){
                    if (this->company == other->company){
                        if (this->actors.size() > 0 && other->actors.size() > 0){
                            if (this->actors.at(0) == other->actors.at(0)){
                                same = true;
                            }
                        } else {
                            same = true;
                        }
                    }
                }
            }
        }
    }
    return same;
}

bool Scene::equals(const Scene &other) const {
    bool same = false;
    if (this->title == other.title){
        if (this->size == other.size){
            if (this->length == other.length){
                if (this->height == other.height){
                    if (this->company == other.company){
                        if (this->actors.size() > 0 && other.actors.size() > 0){
                            if (this->actors.at(0) == other.actors.at(0)){
                                same = true;
                            }
                        } else {
                            same = true;
                        }
                    }
                }
            }
        }
    }
    return same;
}
