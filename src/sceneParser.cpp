#include "sceneParser.h"
#include "FilePath.h"
#include "Rating.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <sys/stat.h>
sceneParser::sceneParser(){}
sceneParser::sceneParser(FilePath f):file(f){}
sceneParser::~sceneParser(){}

QString sceneParser::sysCall(QString cmd){
    QString output("");
    char buffer[4096];
    FILE *pipe = popen(qPrintable(cmd), "r");
    if (!pipe){
        std::string error = QString("popen() failed on command '%1'").arg(cmd).toStdString();
        throw std::runtime_error(error);
    }
    try{    while(!feof(pipe))  {   if(fgets(buffer, 4096, pipe) != NULL){  output.append(buffer);  }   }   }
    catch (...){    pclose(pipe);   throw;  }
    pclose(pipe);
    return output;
}


void sceneParser::parse(FilePath f){
    QString fullpath = f.absolutePath();
    QFileInfo file(fullpath);
    // Add the file info.
    this->file          = f;
    QString currName      = f.getName();
    QString currPath      = f.getPath();
    // Use QFileInfo
    this->created       = file.created().date();
    this->accessed      = file.lastRead().date();
    this->size          = file.size()/1000;
    // Parse out the Name
    this->actors        = parseActors(currName);
    this->title         = parseTitle(currName);
    this->company       = parseCompany(currName);
    this->sceneNumber   = parseSceneNumber(currName);
    // get List of tags, as well as Rating, and try for release date & series name.
    this->tags          = parseTags(currName);
    // get Height, Width, Length, and try for release date.
    bashScript(f);
}

// Get the Title.
QString sceneParser::parseTitle(QString name){
    int leftIndex = 0;
    int rightIndex = name.size();
    // Get the rightmost extent of the Title subsection
    if (name.contains(", Scene")){
        rightIndex = name.indexOf(", Scene");
    } else if (name.contains("feat.")) {
        rightIndex = name.indexOf("feat.");
    } else if (name.contains("(")){
        rightIndex = name.lastIndexOf("(");
    }
    QString temp = name.left(rightIndex - 1);

    // Get the leftmost extent of the Title subsection
    if (name.contains(']')){
        leftIndex = name.indexOf(']');
    } else if (name.contains(" - ")){
        leftIndex = name.indexOf(" - ");
    }
    QString final = temp.right(temp.size() - leftIndex);
    return final;
}

// Get Height, Width, and Length from exif.
void sceneParser::bashScript(FilePath f){
    QMap<QString, QString> videoData;
    static const QRegularExpression rx("^([A-Za-z]+):\\s*(.+)$");
    const QString output = sysCall(QString("%1/scripts/collect_exif.sh \"%2\"").arg(FilePath::parentPath()).arg(f.absolutePath()));
    QRegularExpressionMatchIterator it = rx.globalMatch(output);
    while(it.hasNext()){
        QRegularExpressionMatch m = it.next();
        videoData.insert(m.captured(1), m.captured(2));
    }
    if (videoData.contains("Width"))    {   this->width = videoData.value("Width").toInt();                 }
    if (videoData.contains("Height"))   {   this->width = videoData.value("Height").toInt();                }
    if (videoData.contains("Minutes"))  {   this->length += videoData.value("Minutes").toDouble();          }
    if (videoData.contains("Seconds"))  {   this->length += (videoData.value("Seconds").toDouble())/60.0;   }
    if (videoData.contains("Created"))  {   this->release = QDate::fromString(videoData.value("Created"), "yyyy:MM:dd");    }
}


QStringList sceneParser::parseTags(QString name){
    static const QRegularExpression qualityRx("^[0-9]{3,4}p$"), dateRx("\\d{4}\\.\\d{2}\\.\\d{2}");
    static const QRegularExpression ratingRx("^[ABC][-+]*$");
    static const QRegularExpression listRx(".*\\((.+)\\).*");

    // Get the list of comma-separated items between the parentheses
    QRegularExpressionMatch listMatch = listRx.match(name);
    QStringList items = listMatch.captured(1).split(QRegularExpression(",[\\s]?"));

    QStringListIterator it(items);
    qDebug("\nParsing Tags for %s (%d items detected)...", qPrintable(this->currName), items.size());
    int itemNumber = 0;
    while(it.hasNext())
    {
        QString item = it.next();
        QRegularExpressionMatch qMatch = qualityRx.match(item), rMatch = ratingRx.match(item), dMatch = dateRx.match(item);
        if (qMatch.hasMatch()){
            // This is a Pixel Quality
            qDebug("Removing Quality (%s) from list of tags", qPrintable(item));
            items.removeOne(item);
        } else if (dMatch.hasMatch()){
            // This is a Date
            QDate date = QDate::fromString(dMatch.captured(1), "yyyy.MM.dd");
            if ((release.isNull() || !release.isValid() || date < this->release) && date.isValid() && !date.isNull()){
                qDebug("Release Date being set to: %s", qPrintable(item));
                release = date;
            } else {
                qDebug("Disregarding parsed release date of %s in favour of exif-retrieved release date of %s", qPrintable(item), qPrintable(release.toString("yyyy.MM.dd")));
            }
            items.removeOne(item);
        } else if (rMatch.hasMatch() && !it.hasNext()){
            // This is a Rating
            qDebug("Rating: %s", qPrintable(item));
            this->rating = Rating(item);
            items.removeOne(item);
        } else if (itemNumber == 0){
            // This is a Series Title
            this->series = item;
            items.removeOne(item);
            qDebug("Series: %s", qPrintable(item));
        }
        ++itemNumber;
    }
    if (items.size() > 0){
        QString printableList("");
        QStringListIterator it(items);
        while(it.hasNext()){
            printableList.append(it.next());
            if (it.hasNext()){  printableList.append(", "); }
        }
        qDebug("Tags: %s", qPrintable(printableList));
    }
    this->tags = items;
    qDebug("\n");
    return items;
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
        actors.push_back(match.captured(1).remove(QRegularExpression(SPACE_REGEX)));
    }
    // Get any featured actors.
    name.remove(QRegularExpression(".*feat."));
    name.remove(QRegularExpression("\\(.*\\)\\..*"));
    if (name.isEmpty() || name.isNull()){
        QStringList featuredActors = name.split(QRegularExpression("[&,]"));
        foreach(QString temp, featuredActors){
            actors.push_back(temp.remove(QRegularExpression(SPACE_REGEX)));
        }
    }
    return actors;
}

