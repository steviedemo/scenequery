#include "SceneRenamer.h"
#include <QRegularExpression>
#include <QTextStream>
#include <QStringListIterator>
#include <QDateTime>
#include <QDate>
#include "Scene.h"
#define FEAT            "feat."
#define NAME_SEPERATOR  " - "
SceneRenamer::SceneRenamer(ScenePtr scene): current(scene), actors(QVector<QString>(0)), tags(QVector<QString>(0)),
    folder(""), extension(""), newFilename(""), oldFilename(""), title(""), company(""),
    series(""), releaseString(""), rating(""), tagString(""), mainActor(""), featuredActors(""),
    actorCount(0), height(0), sceneNumber(0), sceneOk(false)
{
    scan(scene);
}
SceneRenamer::SceneRenamer(Scene *scene): current(ScenePtr(scene)), actors(QVector<QString>(0)), tags(QVector<QString>(0)),
    folder(""), extension(""), newFilename(""), oldFilename(""), title(""), company(""),
    series(""), releaseString(""), rating(""), tagString(""), mainActor(""), featuredActors(""),
    actorCount(0), height(0), sceneNumber(0), sceneOk(false)
{
    scan(current);
}
void SceneRenamer::scan(ScenePtr scene){
    this->current = scene;
    this->sceneOk = (!current.isNull());
    if (sceneOk){
        QRegularExpression extRx(".+\\.([A-Za-z4]{2,4})");
        this->oldFilename   = scene->getFilename();
        this->title         = scene->getTitle().trimmed();
        this->releaseString = scene->dateString;
        qDebug("Renaming '%s'", qPrintable(oldFilename));
        if (releaseString.isEmpty()){
            QDate released(scene->getReleased());
            this->releaseString = released.toString("yyyy.MM.dd");
            if (!released.isValid()){
                qWarning("Error: Release Date is invalid.");

                QRegularExpression rx(".*\\([A-Za-z0-9,\\s*]*((?:[0-9.]{8}[0-9]{2})+).*");
                QRegularExpressionMatch m = rx.match(scene->getFilename());
                if (m.hasMatch()){
                    this->releaseString = m.captured(1);
                } else {
                    qWarning("Unable to Match the release date out of the title");
                }
            }
            qDebug("Managed to parse scene release date into: %s", qPrintable(releaseString));
        }
        this->company       = current->getCompany().trimmed();
        this->height        = current->getHeight();
        this->rating        = current->getRating().grade();
        this->tags          = current->getTags();
        this->series        = current->getSeries().trimmed();
        this->sceneNumber   = current->getSceneNumber();
        this->folder        = current->getFolder();
        QVector<QString> actorList = current->getActors();
        foreach(QString name, actorList){
            if (!name.isEmpty() && !actors.contains(name)){
                actors << name.trimmed();
            }
        }
        QRegularExpressionMatch m = extRx.match(oldFilename);
        if (m.hasMatch()){
            this->extension = m.captured(1);
        } else {
            qWarning("Error Generating Extension");
        }
    }
}

QString SceneRenamer::getNewFilename(){
    this->newFilename = "";
    QTextStream out(&newFilename);
    /// Start filename with actor name.
    if (actors.size() > 0){
        out << actors.at(0);
    } else {
        out << "Unknown";
    }
    /// Add Dash Separator
    out << " - ";
    /// Add Company if it exists
    if (!this->company.isEmpty()){
        out << "[" << company << "] ";
    }
    /// Add Title
    title.remove(QRegularExpression("feat\\..*"));
    qDebug("Adding Title: '%s'", qPrintable(title));
    out << title;
    /// Add Scene Number if it exists
    if (sceneNumber > 0){
        out << ", Scene #" << sceneNumber;
    }
    /// Add Featured Actors
    QString featuredString = makeFeaturedString(actors);
    if (!featuredString.isEmpty()){
        out << " feat. " << featuredString;
        qDebug("Featured String being added: 'feat. %s'", qPrintable(featuredString));
    }
    QString data = makeBracketString();
    if (!data.isEmpty()){
        out << " (" << data << ")";
    }
    out << "." << extension;
    out.flush();
    return newFilename;
}
QString SceneRenamer::makeFeaturedString(QVector<QString> names) const{
    QString s("");
    if (names.size() > 1){
        QMap<QString, int> actorMap;
        foreach(QString name, names){
            if (!this->newFilename.contains(name)){
                actorMap[name] += 1;
            }
        }
        QTextStream out(&s);
        QMapIterator<QString,int> it(actorMap);
        while(it.hasNext()){
            it.next();
            qDebug("Adding featured actor to new name: %s", qPrintable(it.key()));
            out << it.key();
            if (it.hasNext()){
                out << ", ";
            }
        }
    }
    return s;
}


QString SceneRenamer::makeBracketString(){
    QString dataString("");
    QTextStream out(&dataString);
    if (!series.isEmpty()){
        out << series;
    }
    if (!releaseString.isEmpty()){
        if (!dataString.isEmpty()){
            out << ", ";
        }
        out << releaseString;
    }
    if (height > 0){
        if (!dataString.isEmpty()){
            out << ", ";
        }
        out << height << "p";
    }
    if (!tags.isEmpty()){
        this->tagString = makeTagString(tags);
        if (!dataString.isEmpty()){
            out << ", ";
        }
        out << tagString;
    }
    if (!rating.isEmpty()){
        if (!dataString.isEmpty()){
            out << ", ";
        }
        out << rating;
    }
    return dataString;
}

QString SceneRenamer::makeTagString(QVector<QString> list) const{
    QString s("");
    QTextStream out(&s);
    if (!list.isEmpty()){
        QVectorIterator<QString> it(list);
        while(it.hasNext()){
            out << it.next();
            if (it.hasNext()){
                out << ", ";
            }
        }
    }
    return s;
}

QString SceneRenamer::displayInfo(){
    QString show("");
    QTextStream out(&show);
    out << "Filename:   " << oldFilename << endl;
    out << "Title:      " << title << endl;
    out << "Company:    " << company << endl;
    out << "Quality:    " << height << endl;
    out << "Series:     " << series << endl;
    out << "Scene No:   " << sceneNumber << endl;
    if (!releaseString.isEmpty()){
        out << "Released:   " << releaseString << endl;
    } else {
        out << "Released:   ???" << endl;
    }
    out << "Rating:     " << rating << endl;
    QVectorIterator<QString> it(actors);
    QString actorString("");
    while(it.hasNext()){
        actorString.append(it.next());
        if (it.hasNext()){
            actorString.append(", ");
        }
    }
    out << "Actors:     " << actorString << endl;
    out << "Tags:       " << makeTagString(tags) << endl;
    out << "New Name:   " << getNewFilename() << endl;
    return show;
}
