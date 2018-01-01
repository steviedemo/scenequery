#include "sceneParser.h"
#include "config.h"
#include "FilePath.h"
#include "genericfunctions.h"
#include "Rating.h"
#include "Scene.h"
#include <QFile>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QVideoFrame>
#include <sys/stat.h>
sceneParser::sceneParser():
    parsed(false), currPath(""), currName(""), title(""), company(""), series(""),
    height(0), width(0), size(0), sceneNumber(0), length(0.0),
    release(QDate()), accessed(QDate()), created(QDate()), md5sum(""){
}
sceneParser::sceneParser(FilePath f):
    file(f), parsed(false),
    currPath(""), currName(""), title(""), company(""), series(""),
    height(0), width(0), size(0), sceneNumber(0), length(0.0),
    release(QDate()), accessed(QDate()), created(QDate()), md5sum("")
    {
}
sceneParser::sceneParser(ScenePtr s):
    parsed(false), currPath(""), currName(""), title(""), company(""), series(""),
    height(0), width(0), size(0), sceneNumber(0), length(0.0),
    release(QDate()), accessed(QDate()), created(QDate()), md5sum(""){
    this->file.setFile(s->getFile());
    this->title = s->getTitle();
    this->company = s->getCompany();
    this->height = s->getHeight();
    this->sceneNumber = s->getSceneNumber();
    this->series = s->getSeries();
    this->release = s->getReleased();
    this->tags = s->getTags();
    this->rating = s->getRating();
    this->actors = s->getActors();
    this->md5sum = s->getMd5sum();
    this->newFilename = "";
}

sceneParser::~sceneParser(){
}


QByteArray checksum(const QString &absolutePath){
    qDebug("Calculating Checksum for %s", qPrintable(absolutePath));
    QByteArray md5("");
    QFile f(absolutePath);
    if (!f.exists()){
        qWarning("Error Calculating Checksum - Can't Locate '%s'", qPrintable(absolutePath));
    } else if (!f.open(QFile::ReadOnly)){
        qWarning("Error Opening %s to calculate Checksum", qPrintable(absolutePath));
    } else {

        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&f)){
            md5 = hash.result();
        } else {
            qWarning("Error Adding data to hash for %s", qPrintable(absolutePath));
        }
    }
    return md5;
}



void sceneParser::parse(void){
    if (file.isEmpty()){
        qWarning("Cannot parse empty filepath");
    } else {
        parse(this->file);
    }
}

/** \brief Extract various pieces of data from a file name, and store them in the appropriate data types. */
void sceneParser::parse(FilePath f){
    qDebug("Parsing '%s'", qPrintable(f.absolutePath()));
    QString fullpath = f.absolutePath();
    QFileInfo file(fullpath);
    // Add the file info.
    this->file.setFile(f);
    this->currName      = f.getName();
    this->currPath      = f.getPath();
    // Use QFileInfo
    this->created       = file.created().date();
    this->accessed      = file.lastRead().date();
    this->size          = file.size()/1000;
    // Parse out the Name
    this->title         = parseTitle(currName);
    //this->md5sum        = checksum(fullpath);
    // get List of tags, as well as Rating, and try for release date & series name.
    parseParentheses(f.getName());
    // get Height, Width, Length, and try for release date.
    bashScript(f);
    readMetadata(f.absolutePath());
    parsed = true;
}

/** \brief Use QMediaObject to retrieve various pieces of data from the file */
void sceneParser::readMetadata(QString filepath){
    qDebug("Retriving Metadata");
    this->player = new QMediaPlayer;
    player->setMedia(QUrl::fromLocalFile(filepath));
    availableMetaDataKeys = player->availableMetaData();
}

// Get the Title.
/** \brief Parse several different pieces of information out of the Title block of the filename.
 *  \param QString name:    Filename.
 *  \return QString title:  the title of the scene.
 */
QString sceneParser::parseTitle(QString name){
    if (name.contains(" - ")){
        QString tempStr = QString("%1").arg(name);
        QRegularExpression tempRx("(.+) - .+");
        QRegularExpressionMatch tempMatch = tempRx.match(name);
        if (tempMatch.hasMatch()){
            actors.push_back(tempMatch.captured(1));
        }
    }
    QString s = name, temp("");
    QRegularExpression rx;
    QRegularExpressionMatch m;
    /// Remove text preceding title
    try{
        if (name.contains('[')){
            // Parse out the company if it's here.
            rx.setPattern(".*\\[(.+)\\].*");
            m = rx.match(name);
            if (m.hasMatch()){
                this->company = m.captured(1);
            }
            // remove the preceding text.
            rx.setPattern(".*\\](.+)");
            m = rx.match(name);
            if (m.hasMatch()){
                temp = m.captured(1);
            }
        } else if (name.contains(" - ")){
            rx.setPattern("(.+) - (.+)");
            m = rx.match(name);
            if (m.hasMatch()){
                this->actors.push_back(m.captured(1).trimmed());
                temp = m.captured(2);
            }
        }
    } catch(std::exception &e) {
        qCritical("Exception Caught While Parsing Start of filename: %s", e.what());
    }
    try{
        /// Remove Text after title.
        // Check for Scene Number
        rx.setPattern(".*[Ss]cene #([0-9]+).*");
        m = rx.match(name);
        if (m.hasMatch()){
            bool ok = false;
            int num = m.captured(1).toInt(&ok);
            if (num > 0){
                this->sceneNumber = num;
            }
            temp.remove(QRegularExpression(",?\\s*[Ss]cene #[0-9]+"));
        }
    } catch(std::exception &e) {
        qCritical("Exception Caught While Looking for Scene Number: %s", e.what());
    }

    try{
        // Check for featured Actors
        if (name.contains("feat.")){
            QString substr("");
            if (name.contains("(")){
                rx.setPattern(".*feat\\.\\s?(.+)\\(.*");
            } else {
                rx.setPattern(".*feat\\.\\s?(.+)");
            }
            m = rx.match(name);
            if (m.hasMatch()){
                substr = m.captured(1);
                rx.setPattern("([^,&]+)[&,]?");
                QRegularExpressionMatchIterator it = rx.globalMatch(substr);
                while(it.hasNext()){
                    m = it.next();
                    actors << m.captured(1).trimmed();
                }
            }
            temp.remove(QRegularExpression("feat\\..*"));
        }
    } catch(std::exception &e) {
        qCritical("Exception Caught While Parsing featured Actors: %s", e.what());
    }

    if (name.contains("]") && name.contains("(")){
        qDebug("Using Match Method for Title");
        rx.setPattern(".*\\](.+)\\(.*");
        m = rx.match(name);
        if (m.hasMatch()){
            this->title = m.captured(1);
        }
    } else if (temp.isEmpty()) {
        qDebug("Using Extraction Method for Title");
        QString str = QString("%1").arg(name);
        if (name.contains("]")){
            int index = name.indexOf(']');
            int chars = name.size() - index;
            str = str.right(chars);
        } else if (name.contains(" - ")){
            int index = name.indexOf(" - ");
            str = str.right(name.size() - index);
        }
        if (name.contains("(")){
            str.remove(QRegularExpression("\\(.*"));
        }
        this->title = str;
    } else {
        qDebug("Using incremental Removal for title");
        this->title = temp.trimmed();
    }

    return title;
}

// Get Height, Width, and Length from exif.
void sceneParser::bashScript(FilePath f){
    QMap<QString, QString> videoData;
    static const QRegularExpression rx("^([A-Za-z]+):\\s*(.+)$");
    QString script = QString("%1/scripts/collect_exif.sh \"%2\"").arg(findDataLocation()).arg(f.absolutePath());
    QString output = system_call(script);
    QRegularExpressionMatchIterator it = rx.globalMatch(output);
    while(it.hasNext()){
        QRegularExpressionMatch m = it.next();
        videoData.insert(m.captured(1), m.captured(2));
    }
    qDebug("Bash Script Output:\n%s", qPrintable(output));
    if (videoData.contains("Width"))    {   this->width = videoData.value("Width").toInt();                 }
    if (videoData.contains("Height"))   {   this->height = videoData.value("Height").toInt();                }
    if (videoData.contains("Minutes"))  {   this->length += videoData.value("Minutes").toDouble();          }
    if (videoData.contains("Seconds"))  {   this->length += (videoData.value("Seconds").toDouble())/60.0;   }
    if (videoData.contains("Created"))  {   this->release = QDate::fromString(videoData.value("Created"), "yyyy:MM:dd");    }
}

/** \brief Parse all the Comma-separated values between the parentheses in the filename.
 *  \param QString name: Name of the file.
 */
void sceneParser::parseParentheses(QString name){
    QString data("");
    QRegularExpression rx(".*\\((.+)\\).*");
    QRegularExpression widthRegex("[0-9]+p");
    QRegularExpression dateRegex("([0-9]{4})\\.([0-9]{2})\\.([0-9]{2})");
    QRegularExpression ratingRegex("\\b[ABCR]{1}[\\+\\-]{0,3}\\b");
    try{
        QRegularExpressionMatch m = rx.match(name);
        if (m.hasMatch()){
            data = m.captured(1);
            data.remove(QRegularExpression("[\\(\\)]"));
        } else {
            return;
        }
        QStringList tagList;
        QStringList list = data.split(QChar(','));
        int index = 0;
        foreach(QString item, list){
            QRegularExpressionMatch dateMatch = dateRegex.match(item);
            QRegularExpressionMatch widthMatch = widthRegex.match(item);
            QRegularExpressionMatch ratingMatch = ratingRegex.match(item);
            if (dateMatch.hasMatch()){
                qDebug("Tag: %s - Date", qPrintable(item));
                this->release = QDate::fromString(item, "yyyy.MM.dd");
            } else if (widthMatch.hasMatch()){
                qDebug("Tag: %s - Quality", qPrintable(item));
                this->height = item.remove('p').toInt();
            } else if (ratingMatch.hasMatch()){
                qDebug("Tag: %s - Rating", qPrintable(item));
                this->rating.fromString(item.trimmed());
            } else if (index == 0){
                qDebug("Tag: %s - Series", qPrintable(item));
                this->series = item.trimmed();
            } else {
                qDebug("Tag: %s - Tag", qPrintable(item));
                tagList << item.trimmed();
            }
            ++index;
        }
        this->tags = tagList;
    } catch (std::exception &e){
        qCritical("Exception Caught while Parsing Tags: %s", e.what());
    }

}

int sceneParser::parseSceneNumber(QString name){
    int sceneNo = 0;
    static const QRegularExpression sceneNumberRx(".*Scene #([0-9]{1,2}).*");
    QRegularExpressionMatch m = sceneNumberRx.match(name);
    if (m.hasMatch()){
        int temp = m.captured(1).toInt();
        if (temp > 0){
            sceneNo =  temp;
        }
    }
    return sceneNo;
}

// Parse the release date out of the Name.
QDate sceneParser::parseDateReleased(QString name){
    QDate date;
    static const QRegularExpression rx(".*([0-9]{4}\\.[0-9]{2}\\.[0-9]{2}.*");
    QRegularExpressionMatch match = rx.match(name);
    if (match.hasMatch()){
        date = QDate::fromString(match.captured(1), "yyyy.MM.dd");
    }
    return date;
}

// Get string from within the square brackets.
QString sceneParser::parseCompany(const QString name){
    static const QRegularExpression rx(".*\\[(.+)\\].*");
    QRegularExpressionMatch match = rx.match(name);
    if (match.hasMatch())   return match.captured(1);
    else                    return "";
}

// Get all actors listed in the name
QStringList sceneParser::parseActors(QString name){
    QStringList actors;
    // Get First Actor
    static const QRegularExpression firstActorRx("^([A-Za-z.\\s]+) - .+");
    QRegularExpressionMatch match = firstActorRx.match(name);
    if (match.hasMatch()){
        actors.push_back(match.captured(1).trimmed());
    }
    // Get any featured actors.
    name.remove(QRegularExpression(".*feat."));
    name.remove(QRegularExpression("\\(.*\\)\\..*"));
    if (name.isEmpty() || name.isNull()){
        QStringList featuredActors = name.split(QRegularExpression("[&,]"));
        foreach(QString temp, featuredActors){
            actors.push_back(temp.trimmed());
        }
    }
    return actors;
}
QString sceneParser::displayInfo(){
    QString show("");
    QTextStream out(&show);
    out << "File:       " << file.absolutePath() << endl;
    out << "Checksum:   " << QString(md5sum) << endl;
    out << "Title:      " << title << endl;
    out << "Company:    " << company << endl;
    out << "Quality:    " << height << endl;
    out << "Series:     " << series << endl;
    out << "Scene No:   " << sceneNumber << endl;
    if (release.isValid()){
        out << "Released:   " << release.toString("MMMM d, yyyy") << endl;
    } else {
        out << "Released:   ???" << endl;
    }
    out << "Rating:     " << rating.grade() << endl;
    QStringListIterator it(actors);
    QString actorString("");
    while(it.hasNext()){
        actorString.append(it.next());
        if (it.hasNext()){
            actorString.append(", ");
        }
    }
    out << "Actors:     " << actorString << endl;
    it = tags;
    QString tagString("");
    while(it.hasNext()){
        tagString += it.next() + (it.hasNext() ? ", " : "");
    }
    out << "Tags:       " << tagString << endl;
    out << "New Name:   " << formatFilename() << endl;
    out << "Available Metadata Tags: " << endl;
    QString keys("");
    it = availableMetaDataKeys;
    while(it.hasNext()){
        keys += it.next() + (it.hasNext() ? ", " : "");
    }
    out << keys;
    return show;
}
/** **********************************************************************
 * \brief Functions for formatting the different sections of a filename. */
QString sceneParser::formatFilename()   {   return QString("%1%2%3%4.%5").arg(formatActor()).arg(formatCompany()).arg(formatTitle()).arg(formatParentheses()).arg(file.getExtension()); }
QString sceneParser::formatActor()      {   return (actors.isEmpty() ? "Unknown - " : QString("%1 - ").arg(actors.at(0)));  }
QString sceneParser::formatCompany()    {   return (company.isEmpty() ? "" : QString("[%1] ").arg(company));      }
QString sceneParser::formatTitle(){
    QString data("");
    int added = 0;
    if (!title.isEmpty()){
        data.append(title);
    }
    if (sceneNumber > 0){
        if (added > 0){
            data.append(",");
        }
        data += " Scene #" + QString::number(sceneNumber);
        added++;
    }
    if (actors.size() > 1){
        data += " feat. ";
        QStringListIterator it(actors);
        it.next();  // Skip First actor
        while(it.hasNext()){
            QString actor = it.next();
            if (!actor.isEmpty()){
                data += actor + (it.hasNext() ? ", " : "");
            }
        }
        added++;
    }
    return data;
}

QString sceneParser::formatParentheses(){
    QString data("");
    int added = 0;
    // Add Series
    if (!this->series.isEmpty()){
        data.append(series);
        added++;
    }
    // add Release Date
    if (!this->release.isNull() && this->release.isValid()){
        if (added > 0){
            data.append(", ");
        }
        data.append(release.toString("yyyy.MM.dd"));
        added++;
    }
    // Add Quality
    if (height > 0){
        if (added > 0){
            data.append(", ");
        }
        data += QString("%1p").arg(height);
        added++;
    }
    // Add Tags
    if (!tags.isEmpty()){
        QString tagString("");
        QStringListIterator it(tags);
        while(it.hasNext()){
            QString tag = it.next();
            if (!tag.isEmpty()){
                tagString += tag + (it.hasNext() ? ", " : "");
            }
        }
        if (!tagString.isEmpty()){
            if (added > 0){
                data.append(", ");
            }
            data.append(tagString);
            added++;
        }
    }
    // Add Rating
    if (!rating.isEmpty()){
        if (added > 0){
            data.append(", ");
        }
        data.append(rating.grade());
    }
    QString returnString("");
    if (!data.isEmpty()){
        returnString = QString(" (%1)").arg(data);
    }
    return returnString;
}

