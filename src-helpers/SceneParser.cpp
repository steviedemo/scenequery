#include "SceneParser.h"
#include "config.h"
#include "filenames.h"
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
SceneParser::SceneParser():
    parsed(false), title(""), company(""), series(""),
    height(0), width(0), size(0), sceneNumber(0), length(QTime(0, 0, 0)),
    release(QDate()), accessed(QDate()), created(QDate()), md5sum(""){
    this->file.first="";
    this->file.second="";
}
SceneParser::SceneParser(QPair<QString,QString>file):
    parsed(false), title(""), company(""), series(""),
    height(0), width(0), size(0), sceneNumber(0), length(QTime(0, 0, 0)),
    release(QDate()), accessed(QDate()), created(QDate()), md5sum("")
{
    this->file.first = file.first;
    this->file.second = file.second;
}
SceneParser::SceneParser(QString path):
    parsed(false), title(""), company(""), series(""),
    height(0), width(0), size(0), sceneNumber(0), length(QTime(0, 0, 0)),
    release(QDate()), accessed(QDate()), created(QDate()), md5sum("")
{
    this->file = splitAbsolutePath(path);
}


SceneParser::~SceneParser(){
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

void SceneParser::parse(void){
    if (file.first.isEmpty() || file.second.isEmpty()){
        qWarning("Cannot parse empty filepath");
    } else {
        this->parse(file);
    }
}
void SceneParser::parse(QPair<QString,QString> path){
    this->file = path;
    QString absolutePath = QString("%1/%2").arg(file.first).arg(file.second);
    QFileInfo info(absolutePath);
    this->created   = info.created().date();
    this->accessed  = info.lastRead().date();
    this->size      = info.size();
    if (size < 0){
        this->size = (-1)*(this->size);
    }
    this->title     = this->parseTitle(file.second).trimmed();
    parseParentheses(file.second);
    bashScript(absolutePath);
    doubleCheckNames();
    this->parsed = true;
}

/** \brief Extract various pieces of data from a file name, and store them in the appropriate data types. */
void SceneParser::parse(QString absolutePath){
    qDebug("Parsing '%s'", qPrintable(absolutePath));
    this->file = splitAbsolutePath(absolutePath);
    // Add the file info.
    QFileInfo info(absolutePath);
    // Use QFileInfo
    this->created       = info.created().date();
    this->accessed      = info.lastRead().date();
    this->size          = info.size()/1000;
    if (size < 0){
        this->size = (-1)*(this->size);
    }
    // Parse out the Name
    this->title         = parseTitle(file.second).trimmed();
    //this->md5sum        = checksum(fullpath);
    // get List of tags, as well as Rating, and try for release date & series name.
    parseParentheses(file.second);
    // get Height, Width, Length, and try for release date.
    bashScript(absolutePath);
    doubleCheckNames();
    //readMetadata(f.absolutePath());
    parsed = true;
}

/** \brief Use QMediaObject to retrieve various pieces of data from the file */
void SceneParser::readMetadata(QString filepath){
    //qDebug("Retriving Metadata");
    this->player = new QMediaPlayer;
    player->setMedia(QUrl::fromLocalFile(filepath));
    availableMetaDataKeys = player->availableMetaData();
}

// Get the Title.
/** \brief Parse several different pieces of information out of the Title block of the filename.
 *  \param QString name:    Filename.
 *  \return QString title:  the title of the scene.
 */
QString SceneParser::parseTitle(QString name){
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
        //qDebug("Using Match Method for Title");
        rx.setPattern(".*\\](.+)\\(.*");
        m = rx.match(name);
        if (m.hasMatch()){
            QString str = m.captured(1);
            str.remove(QRegularExpression("\\s*feat\\..*"));
            this->title = str.trimmed();
        }
    } else if (temp.isEmpty()) {
        //qDebug("Using Extraction Method for Title");
        QString str = QString("%1").arg(name);
        if (name.contains("]")){
            int index = name.indexOf(']');
            int chars = name.size() - index;
            str = str.right(chars);
        } else if (name.contains(" - ")){
            int index = name.indexOf(" - ");
            str = str.right(name.size() - index);
        }
        str.remove(QRegularExpression("\\s*\\(.*"));
        str.remove(QRegularExpression("\\s*feat\\..*"));
        this->title = str.trimmed();
    } else {
        //qDebug("Using incremental Removal for title");
        this->title = temp.trimmed();
    }

    return title;
}

// Get Height, Width, and Length from exif.
void SceneParser::bashScript(QString fileToAnalyze){
    QMap<QString, QString> videoData;
    static const QRegularExpression rx("([A-Za-z]+): ([0-9:]+)");
    QString output("");
    QString script = QString("%1/scripts/collect_exif.sh").arg(findDataLocation());
    try{
        if (system_call_blocking(script, QStringList() << fileToAnalyze, output)){
            qDebug("Bash Script Output:\n%s\n", qPrintable(output));
            QRegularExpressionMatchIterator it = rx.globalMatch(output);
            while(it.hasNext()){
                QRegularExpressionMatch m = it.next();
                //qDebug("Bash Script output parameter: key: '%s', value: '%s'", qPrintable(m.captured(1)), qPrintable(m.captured(2)));
                videoData.insert(m.captured(1), m.captured(2));
            }
            //qDebug("Bash Script Output:\n%s", qPrintable(output));
            if (videoData.contains("Width"))    {   this->width = videoData.value("Width").toInt();                 }
            if (videoData.contains("Height"))   {   this->height = videoData.value("Height").toInt();                }
            QRegularExpression lengthRx(".*Length:\\s*([0-9]:[0-9]{2}:[0-9]{2}).*");

            if (videoData.contains("Length")) {
                QString time = videoData.value("Length");
                //qDebug("Bash script returned a length of '%s'", qPrintable(time));
                length = QTime::fromString(time, "h:mm:ss");
                //qDebug("Time Object contains a value of: %s", qPrintable(length.toString("h:mm:ss")));
            } else {
                qWarning("Did Not find 'Length' in the bash script output");
            }

            //if (videoData.contains("Created"))  {   this->release = QDate::fromString(videoData.value("Created"), "yyyy:MM:dd");    }
        } else {
            qWarning("Error Running Bash Script, '%s'", qPrintable(script));
        }
    } catch (std::exception &e){
        qWarning("Caught Exception while running exif_tool script on '%s': %s", qPrintable(fileToAnalyze), e.what());
    }
}

/** \brief Parse all the Comma-separated values between the parentheses in the filename.
 *  \param QString name: Name of the file.
 */
void SceneParser::parseParentheses(QString name){
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
                //qDebug("Tag: %s - Date", qPrintable(item));
                this->release = QDate::fromString(item, "yyyy.MM.dd");
            } else if (widthMatch.hasMatch()){
                //qDebug("Tag: %s - Quality", qPrintable(item));
                this->height = item.remove('p').toInt();
            } else if (ratingMatch.hasMatch()){
                //qDebug("Tag: %s - Rating", qPrintable(item));
                this->rating.fromString(item.trimmed());
            } else if (index == 0){
                //qDebug("Tag: %s - Series", qPrintable(item));
                this->series = item.trimmed();
            } else {
                //qDebug("Tag: %s - Tag", qPrintable(item));
                tagList << item.trimmed();
            }
            ++index;
        }
        this->tags = tagList;
    } catch (std::exception &e){
        qCritical("Exception Caught while Parsing Tags: %s", e.what());
    }

}

int SceneParser::parseSceneNumber(QString name){
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
QDate SceneParser::parseDateReleased(QString name){
    QDate date;
    static const QRegularExpression rx(".*([0-9]{4}\\.[0-9]{2}\\.[0-9]{2}.*");
    QRegularExpressionMatch match = rx.match(name);
    if (match.hasMatch()){
        date = QDate::fromString(match.captured(1), "yyyy.MM.dd");
    }
    return date;
}

// Get string from within the square brackets.
QString SceneParser::parseCompany(const QString name){
    static const QRegularExpression rx(".*\\[(.+)\\].*");
    QRegularExpressionMatch match = rx.match(name);
    if (match.hasMatch())   return match.captured(1);
    else                    return "";
}

// Get all actors listed in the name
QStringList SceneParser::parseActors(QString name){
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
