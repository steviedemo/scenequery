#include "Scene.h"
#include "Rating.h"
#include "filenames.h"
#include "genericfunctions.h"
#include "SceneParser.h"
#include "sql.h"
#include "query.h"
#include <QSqlQuery>
#include <QVariant>
#include <QStringListIterator>
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


Scene::Scene():Entry(),
    ID(0), length(QTime(0,0,0)), height(0), width(0), sceneNumber(0), size(0),
    added(QDate()), released(QDate()), opened(QDate()),
    title(""), company(""), series(""), url(""), dateString(""),md5sum(""){
    this->ages = {};
    displayBuilt = false;
    this->file.first = "";
    this->file.second = "";
}
Scene::Scene(QString absolutePath): Entry(), ID(0),
    length(QTime(0,0,0)), height(0), width(0), sceneNumber(0), size(0),
    added(QDate()), released(QDate()), opened(QDate()),
    title(""), company(""), series(""), url(""), dateString("")
{
    this->file = splitAbsolutePath(absolutePath);
    sceneParser p;
    p.parse(file);
    this->fromParser(p);
    displayBuilt = false;
}

Scene::Scene(pqxx::result::const_iterator record):Entry(), ID(0),
    length(QTime(0,0,0)), height(0), width(0), sceneNumber(0), size(0),
    added(QDate()), released(QDate()), opened(QDate()),
    title(""), company(""), series(""), url(""), dateString(""){
    this->ages = {};
    this->fromRecord(record);
    displayBuilt = false;
}

Scene::Scene(sceneParser p):Entry(), ID(0),
    length(QTime(0,0,0)), height(0), width(0), sceneNumber(0), size(0),
    added(QDate()), released(QDate()), opened(QDate()),
    title(""), company(""), series(""), url(""), dateString(""), md5sum(""){
    this->ages = {};
    this->fromParser(p);
    displayBuilt = false;
}
Scene::Scene(const Scene &s):Entry(), ID(s.ID),
    length(s.length), height(s.height), width(s.width), sceneNumber(s.sceneNumber), size(s.size),
    added(s.added), released(s.released), opened(s.opened),
    title(s.title), company(s.company), series(s.series), url(s.url), dateString(s.dateString){
    this->actors = s.actors;
    this->tags = s.tags;
    this->ages = s.ages;
    this->displayBuilt = false;
//    this->itemActors = s.itemActors;
//    this->itemCompany = s.itemCompany;
//    this->itemDate = s.itemDate;
//    this->itemFeaturedActors = s.itemFeaturedActors;
//    this->itemLength = s.itemLength;
//    this->itemP
}

void Scene::fromParser(sceneParser p){
    if (!p.isEmpty()){
        if (!p.isParsed()){
            p.parse();
        }
        this->file.first = p.getFilepath();
        this->file.second = p.getFilename();
        //qDebug("Making Scene with name: '%s', and path: '%s'", qPrintable(file.second), qPrintable(file.first));
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
        ages = {};
    }
    displayBuilt = false;
}


Scene::Scene(QSqlRecord r){
    this->ID        = r.value("id").toLongLong();
    this->file.second  = r.value("filename").toString();
    this->file.first  = r.value("filepath").toString();
    this->title     = r.value("title").toString();
    this->company   = r.value("company").toString();
    this->series    = r.value("series").toString();
    this->rating.fromString(r.value("rating").toString());
    this->tags      = r.value("tags").toString().split(',');
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
    if(!a.isEmpty() && !this->actors.contains(a)){
        this->actors << a;
    }
}
void Scene::removeActor(QString a){
    if (!a.isEmpty() && a.contains(a)){
        this->actors.removeOne(a);
    }
}

void Scene::renameActor(QString oldName, QString newName){
    if (!oldName.isEmpty() && !newName.isEmpty() && actors.contains(oldName)){
        actors.removeOne(oldName);
        actors.prepend(newName);
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
            file.first = QString::fromStdString(entry["filepath"].as<string>());
        }
        if (!entry.at("filename").is_null()){
            file.second = QString::fromStdString(entry["filename"].as<string>());
        }
        if (!file.second.isEmpty() && !file.first.isEmpty()){
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
            }
        }
        if (!entry.at("created").is_null()){
            QString temp = QString::fromStdString(entry["created"].as<std::string>());
            dateMatch = dateRegex.match(temp);
            if (dateMatch.hasMatch()){
                this->released = QDate::fromString(temp, "yyyy-MM-dd");
                this->dateString = temp.replace('-', '.');
            }
        }
        if (!entry.at("accessed").is_null()){
            QString temp = QString::fromStdString(entry["accessed"].as<std::string>());
            dateMatch = dateRegex.match(temp);
            if (dateMatch.hasMatch()){
                this->opened = QDate::fromString(temp, "yyyy-MM-dd");
            }
        }
        if (!entry.at("tags").is_null()){
            this->tags = QString::fromStdString(entry["tags"].as<std::string>()).split(',', QString::SkipEmptyParts);
        }
        bool error = false;
        for (int idx = 0; idx < 4 && !error; ++idx){
            try{
                std::string fieldname = qPrintable(QString("actor%1").arg(idx+1));
                std::string fieldage  = qPrintable(QString("age%1").arg(idx+1));
                if (!entry.at(fieldname).is_null()){
                    actors << QString::fromStdString(entry[fieldname].as<std::string>());
                } else {
                    actors << "";
                }
                if (!entry.at(fieldage).is_null()){
                    ages.push_back(entry[fieldage].as<int>());
                } else {
                    ages.push_back(0);
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

Scene::~Scene(){}

void Scene::setFile(const QString &absolutePath){
    this->file = splitAbsolutePath(absolutePath);
}

bool Scene::exists() const{
    QString absolutePath = QString("%1/%2").arg(file.first).arg(file.second);
    QFile file(absolutePath);
    return file.exists();
}

bool Scene::hasDisplay(){
    return displayBuilt;
}

ItemList Scene::getItemList(){
    this->displayRow << itemActors << itemTitle << itemCompany << itemQuality << itemSize << itemLength << itemDate << itemRating;
    return displayRow;
}

QList<QStandardItem *> Scene::getQStandardItem(){
    if (!displayBuilt){
        qDebug("Building new Scene Item for %s", qPrintable(title));
        return this->buildQStandardItem();
    } else {

    }
    row << itemActors.data() << itemTitle.data() << itemCompany.data() << itemQuality.data() << itemSize.data() << itemLength.data() << itemDate.data() << itemRating.data();
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
    this->itemSize = ItemPtr(new QStandardItem(sizeString));
    this->itemTitle = ItemPtr(new QStandardItem());
    this->itemTitle->setData(QVariant(title), Qt::DisplayRole);
    this->itemCompany = ItemPtr(new QStandardItem());
    this->itemCompany->setData(QVariant(company), Qt::DisplayRole);
    QString path = QString("%1/%2").arg(file.first).arg(file.second);
    this->itemPath = ItemPtr(new QStandardItem(path));
    QString date("");
    if (released.isValid()){
        date = released.toString("yyyy/MM/dd");
    }
    this->itemDate = ItemPtr(new QStandardItem(date));
    this->itemQuality = ItemPtr(new QStandardItem());
    if (height > 0){
        this->itemQuality->setData(QVariant(height), Qt::DecorationRole);
    } else {
        itemQuality->setText("");
    }
    this->itemLength = ItemPtr(new QStandardItem(length.toString("h:mm:ss")));
    this->itemRating = ItemPtr(new QStandardItem(rating.grade()));
    QString mainActor(""), featuredActors("");
    if (actors.size() > 0){
        mainActor = actors.at(0).trimmed();
        if (actors.size() > 1){
            QStringListIterator it(actors);
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
    this->itemActors = ItemPtr(new QStandardItem());
    this->itemActors->setData(QVariant(mainActor), Qt::DisplayRole);
    this->itemFeaturedActors = ItemPtr(new QStandardItem());
    this->itemFeaturedActors->setData(QVariant(featuredActors), Qt::DisplayRole);
    row << itemActors.data() << itemTitle.data() << itemCompany.data() << itemQuality.data() \
        << itemFeaturedActors.data() <<  itemSize.data() << itemLength.data() \
        << itemDate.data() << itemRating.data() << itemPath.data();
    displayBuilt = true;
    return row;
}

void Scene::updateQStandardItem(){
    if (displayBuilt){
        QString mainActor(""), featuredActors("");
        if (actors.size() > 0){
            mainActor = actors.at(0).trimmed();
            if (actors.size() > 1){
                QStringListIterator it(actors);
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
    QStringListIterator it(tags);
    while(it.hasNext()){
        out << it.next() << (it.hasNext() ? ", " : "");
    }
    return s;
}
/*
void Scene::setLength(double minutes){
    int seconds = (int)(60*minutes);
    QTime temp(0,0,0);
    this->length = temp.addSecs(seconds);
}
*/
Query Scene::toQuery() const{
    Query q;
    q.setTable("scenes");
    q.addCriteria("ID", QString::number(this->ID));
    q.add("FILEPATH", file.first);
    q.add("FILENAME", file.second);
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
QDate Scene::getReleased() const{
    return released;
}
QString Scene::getReleaseString() const{

    if (this->dateString.isEmpty()){
        return released.toString("yyyy.MM.dd");
    }
    return dateString;
}
