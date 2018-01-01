#include "Scene.h"
#include "Rating.h"
#include "genericfunctions.h"
#include "sceneParser.h"
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


Scene::Scene():Entry(),
    ID(0), file(FilePath()), length(0.0), height(0), width(0), size(0), sceneNumber(0),
    added(QDate()), released(QDate()), opened(QDate()),
    title(""), company(""), series(""), url(""), md5sum(""){
    this->ages = {};
    displayBuilt = false;
}
Scene::Scene(sceneParser p):Entry(),
    ID(0), file(FilePath()), length(0.0), height(0), width(0), size(0), sceneNumber(0),
    added(QDate()), released(QDate()), opened(QDate()),
    title(""), company(""), series(""), url(""), md5sum(""){
    this->ages = {};
    this->fromParser(p);
    displayBuilt = false;
}

Scene::Scene(FilePath file) : Entry(),
    ID(0), file(file), length(0.0), height(0), width(0), size(0), sceneNumber(0),
    added(QDate()), released(QDate()), opened(QDate()),
    title(""), company(""), series(""), url(""), md5sum(""){
    this->ages = {};
    sceneParser p;
    p.parse(file);
    this->fromParser(p);
    displayBuilt = false;
}
void Scene::fromParser(sceneParser p){
    if (!p.isEmpty()){
        if (!p.isParsed()){
            p.parse();
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
        ages = {};
    }
    displayBuilt = false;
}


Scene::Scene(QSqlRecord r){
    this->ID        = r.value("id").toLongLong();
    this->file      = FilePath(r.value("filepath").toString(), r.value("filename").toString());
    this->title     = r.value("title").toString();
    this->company   = r.value("company").toString();
    this->series    = r.value("series").toString();
    this->rating.fromString(r.value("rating").toString());
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
    displayBuilt = false;
}

Scene::Scene(pqxx::result::const_iterator &i):Entry(),
    ID(0), file(FilePath()), length(0.0), height(0), width(0), size(0), sceneNumber(0),
    added(QDate()), released(QDate()), opened(QDate()),
    title(""), company(""), series(""), url(""){
    this->ages = {};
    this->fromRecord(i);
    displayBuilt = false;
}

void Scene::fromRecord(pqxx::result::const_iterator &i){
    try{
        QRegularExpression dateRegex("[0-9]{4}\\.[0-9]{2}\\.[0-9]{2}");
        QRegularExpressionMatch dateMatch;
        trace();
        if (!i["id"].is_null())     {    this->ID = i["id"].as<int>();   }
        trace();
        if (!i["filename"].is_null() && !i["filepath"].is_null()){
            QString filename = QString::fromStdString(i["filename"].as<string>());
            QString filepath = QString::fromStdString(i["filepath"].as<string>());
            this->setFile(FilePath(filepath, filename));
        }
        trace();
        if (!i["title"].is_null())  {   this->title     = QString::fromStdString(i["title"].as<std::string>());     }
        trace();
        if (!i["company"].is_null()){   this->company   = QString::fromStdString(i["company"].as<std::string>());   }
        trace();
        if (!i["series"].is_null()) {   this->series    = QString::fromStdString(i["series"].as<std::string>());    }
        trace();
        if (!i["scene_no"].is_null()){  this->sceneNumber = (i["scene_no"].as<int>());                              }
        trace();
        if (!i["rating"].is_null()) {   this->rating.fromString(QString::fromStdString(i["rating"].as<std::string>()));                    }
        trace();
        if (!i["size"].is_null())   {   this->size      = i["size"].as<int>();      }
        trace();
        if (!i["length"].is_null()) {   this->length    = i["length"].as<float>();  }
        trace();
        if (!i["width"].is_null())  {   this->width     = i["width"].as<int>();     }
        trace();
        if (!i["height"].is_null()) {   this->height    = i["height"].as<int>();    }
        trace();
        if (!i["added"].is_null())  {
            trace();
            QString temp = QString::fromStdString(i["added"].as<std::string>());
            trace();
            dateMatch = dateRegex.match(temp);
            if (dateMatch.hasMatch()){
                this->added = QDate::fromString(temp, "yyyy-MM-dd");
            }
        }
        trace();
        if (!i["created"].is_null()){
            trace();
            QString temp = QString::fromStdString(i["created"].as<std::string>());
            dateMatch = dateRegex.match(temp);
            if (dateMatch.hasMatch()){
                this->released = QDate::fromString(temp, "yyyy-MM-dd");
            }
        }
        trace();
        if (!i["accessed"].is_null()){
            QString temp = QString::fromStdString(i["accessed"].as<std::string>());
            dateMatch = dateRegex.match(temp);
            if (dateMatch.hasMatch()){
                this->opened = QDate::fromString(temp, "yyyy-MM-dd");
            }
        }
        bool error = false;
        for (int idx = 0; idx < 4 && !error; ++idx){
            try{
                std::string fieldname = qPrintable(QString("actor%1").arg(idx+1));
                std::string fieldage  = qPrintable(QString("age%1").arg(idx+1));
                if (!i[fieldname].is_null()){
                    actors << QString::fromStdString(i[fieldname].as<std::string>());
                }
                if (!i[fieldage].is_null()){
                    ages.push_back(i[fieldage].as<int>());
                } else {
                    ages.push_back(0);
                }
            } catch (std::out_of_range &e){
                qWarning("Error: index %d out of range - %s", idx, e.what());
                error = true;
            }
        }
        trace();
//        if (!i["url"].is_null()){
//            this->url = QString::fromStdString(i["url"].as<std::string>());
//        }
        trace();
    }catch(std::exception &e){
        qWarning("Exception Caught while making Scene: %s", e.what());
    } catch (...){
        qWarning("Unknown Exception Caught While Making Scene");
    }
}

Scene::~Scene(){}

void Scene::setMd5sum(){
    this->md5sum = checksum(this->file.absolutePath());
}

bool Scene::hasDisplay(){
    return displayBuilt;
}

Scene::RowData Scene::getRowData(){
    Scene::RowData data;
    data.filename = this->file.getName();
    data.filepath = this->file.getPath();
    data.title = this->title;
    QString main(""), feat("");
    if (this->actors.size() > 0){
        main = actors.at(0);
        if (this->actors.size() > 1){
            QStringListIterator it(actors);
            while(it.hasNext()){
                feat += it.next() + (it.hasNext() ? ", " : "");
            }
        }
    }
    data.mainActor = main;
    data.featured = feat;
    data.date = this->released.toString("MMMM d, yyyy");
    data.rating = rating.grade();
    data.company = this->company;
    data.series = this->series;
    if (this->height > 0){
        data.quality = QString("%1p").arg(this->height);
    } else {
        data.quality = "";
    }
    double megabyte = (1024*1024*1024.0);
    double gigabyte = (1024*megabyte);
    QString sizeString("");
    if (this->size > (gigabyte)){
        double sizeDouble = (double)(size/gigabyte);
        sizeString = QString::number(sizeDouble, 'f', 2) + " GB";
    } else {
        double sizeDouble = (double)(size/megabyte);
        sizeString = QString::number(sizeDouble, 'f', 2) + " MB";
    }
    data.size = sizeString;
    qint64 seconds = (qint64)(length*60);
    QTime time = QTime(0,0,0);
    time = time.addSecs(seconds);
    if (length > 60.0){
        data.length = QString("%1 h, %2 min, %3 sec").arg(time.hour()).arg(time.minute()).arg(time.second());
    } else if (length > 1.00){
        data.length = QString("%1 min, %2 sec").arg(time.minute()).arg(time.second());
    } else {
        data.length = QString("%1 seconds").arg(time.second());
    }
    return data;
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
    double megabyte = (1024*1024*1024.0);
    double gigabyte = (1024*megabyte);
    QString sizeString("");
    if (this->size > (gigabyte)){
        double sizeDouble = (double)(size/gigabyte);
        sizeString = QString::number(sizeDouble, 'f', 2) + " GB";
    } else {
        double sizeDouble = (double)(size/megabyte);
        sizeString = QString::number(sizeDouble, 'f', 2) + " MB";
    }
    this->itemTitle = ItemPtr(new QStandardItem());
    this->itemTitle->setData(QVariant(title), Qt::DisplayRole);
    this->itemCompany = ItemPtr(new QStandardItem());
    this->itemCompany->setData(QVariant(company), Qt::DisplayRole);
    this->itemSize = ItemPtr(new QStandardItem(sizeString));
    QString date("");
    if (released.isValid()){
        date = released.toString("yyyy/MM/dd");
    }
    this->itemDate = ItemPtr(new QStandardItem(date));
    QString quality = ((height > 0) ? QString("%1p").arg(height) : "");
    this->itemQuality = ItemPtr(new QStandardItem(quality));
    qint64 seconds = (qint64)(length*60);
    QTime time = QTime(0,0,0);
    time = time.addSecs(seconds);
    QString lenStr("");
    if (length > 60.0){
        lenStr = QString("%1 h, %2 min, %3 sec").arg(time.hour()).arg(time.minute()).arg(time.second());
    } else if (length > 1.00){
        lenStr = QString("%1 min, %2 sec").arg(time.minute()).arg(time.second());
    } else {
        lenStr = QString("%1 seconds").arg(time.second());
    }
    this->itemLength = ItemPtr(new QStandardItem(lenStr));
    this->itemRating = ItemPtr(new QStandardItem(rating.grade()));
    QString mainActor(""), featuredActors("");
    if (actors.size() > 0){
        mainActor = actors.at(0).trimmed();
        if (actors.size() > 1){
            QStringListIterator it(actors);
            it.next();
            while(it.hasNext()){
                featuredActors += it.next() + (it.hasNext() ? ", " : "");
            }
        }
    }
    this->itemActors = ItemPtr(new QStandardItem());
    this->itemActors->setData(QVariant(mainActor), Qt::DisplayRole);
    this->itemFeaturedActors = ItemPtr(new QStandardItem());
    this->itemFeaturedActors->setData(QVariant(featuredActors), Qt::DisplayRole);
    row << itemActors.data() << itemTitle.data() << itemCompany.data() << itemQuality.data() << itemFeaturedActors.data() <<  itemSize.data() << itemLength.data() << itemDate.data() << itemRating.data();
    displayBuilt = true;
    return row;
}

void Scene::updateQStandardItem(){
    if (displayBuilt){
        QString s("");
        int index = 0;
        foreach(QString a, actors){
            s += a + ((index+1) == actors.size() ? ", " : "");
        }
        this->itemActors->setText(s);
        QString date("");
        if (released.isValid()){
            date = released.toString("yyyy/MM/dd");
        }
        this->itemDate->setText(date);
        QString quality = ((height > 0) ? QString("%1p").arg(height) : "");
        this->itemQuality->setText(quality);
        this->itemTitle->setText(title);
        this->itemLength->setText(QString::number(length, 'f', 2));
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

Query Scene::toQuery() const{
    Query q;
    q.setTable("scenes");
    q.add("TITLE", title);
    q.add("COMPANY", company);
    q.add("SERIES", series);
    q.add("RATING", rating.grade());
    q.add("SCENE_NO", sceneNumber);
    q.add("SIZE",   size);
    q.add("LENGTH", length);
    q.add("WIDTH", width);
    q.add("HEIGHT", height);
    q.add("TAGS",  tagString());
    q.add("TAGS",  listToString(this->tags));
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

int Scene::entrySize(){
    int size = 0;
    size += (this->file.isEmpty() ? 0 : 1);
    size += (this->title.isEmpty() ? 0 : 1);
    size += (this->company.isEmpty() ? 0 : 1);
    size += (this->series.isEmpty() ? 0 : 1);
    size += (this->rating.isEmpty() ? 0 : 1);
    size += tags.size();
    size += ((width == 0) ? 0 : 1);
    size += ((height == 0) ? 0 : 1);
    size += ((size == 0) ? 0 : 1);
    size += ((length == 0.0) ? 0 : 1);
    size += (this->added.isValid() ? 0 : 1);
    size += (this->opened.isValid() ? 0 : 1);
    size += (this->released.isValid() ? 0 : 1);
    size += actors.size();
    size += ages.size();
    return size;
}
