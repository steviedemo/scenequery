#include "config.h"
#include "curlTool.h"
#include "filenames.h"
#include "Actor.h"
#include "Scene.h"
#include "definitions.h"
#include "genericfunctions.h"
#include <curl/curl.h>
#include <QtConcurrentRun>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QFutureSynchronizer>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QThreadPool>
#include <QVector>
#define out();  qDebug("%s::%s::%d", __FILE__,__FUNCTION__,__LINE__);
curlTool::curlTool(){
    this->dataPath = findDataLocation();
    this->headshotPath = findHeadshotLocation();
    this->thumbnailPath = findThumbnailLocation();
    this->setTerminationEnabled(true);
}

DownloadThread::DownloadThread(QString name){
    this->name = name;
    this->html = "";
    this->photo = "";
    this->actor = QSharedPointer<Actor>(new Actor(name));
}

curlTool::~curlTool(){}
DownloadThread::~DownloadThread(){}

/** \brief curlThread's Event Loop */
void curlTool::run(){
    this->keepRunning = true;
    qDebug("curl Thread Started");
    while(keepRunning){
        ;
    }
    qDebug("curl thread Stopping");
}


/** \brief DownloadThread's Main Routine */
void DownloadThread::run(){
    qDebug("Download Thread Started with '%s'", qPrintable(name));
    this->actor = ActorPtr(new Actor(name));
    if (!name.isEmpty()){
        Biography b(name);
        getFreeonesData(name, &b);
        bool iafdProfileExists = getIAFDData(name, &b);
        qDebug("%s got bio of size %d", qPrintable(name), b.size());
        actor->setBio(b);
        if (iafdProfileExists){
            actor->setHeadshot(downloadHeadshot(name));
        }
        qDebug("Sending Actor Object for '%s'", qPrintable(name));
        emit sendActor(actor);
    }
    qDebug("Thread Finished for '%s'", qPrintable(name));
    emit finished();
}

void curlTool::stopThread(){
    this->keepRunning = false;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

QString request(QString urlString){
    CURL *curl;
    QString html("");
   // char curl_errbuf[CURL_ERROR_SIZE];
    CURLcode res;
    std::string readBuffer;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, qPrintable(urlString));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    html = QString::fromStdString(readBuffer);
    return html;
}

QString DownloadThread::getHTML(Website w, QString name){
    QString html(""), url("");
    QTextStream output(&url);
    QString nameCopy_1 = QString("%1").arg(name);
    QString nameCopy_2 = QString("%1").arg(name);
    nameCopy_1 = nameCopy_1.trimmed();
    nameCopy_2 = nameCopy_2.trimmed();
    QString lc = name.toLower();
    if (w == Freeones){
        nameCopy_1 = nameCopy_1.toLower();
        output << "www.freeones.ca/html/" << nameCopy_1.at(0) << "_links/" << nameCopy_2.remove('.').replace(" ", "_") << "/";
        qDebug("Retrieving Freeones bio from: %s", qPrintable(url));
        html = request(url);
    } else if (w == IAFD) {
        output << "www.iafd.com/person.rme/perfid=" << nameCopy_1.toLower().remove(QRegularExpression("[\\s\\.\\-\\']")) << "/gender=f/" << nameCopy_2.toLower().replace(" ", "-") << ".htm";
        qDebug("Retrieving IAFD Bio from: %s", qPrintable(url));
        html = system_call(QString("curl %1").arg(url));
    }
    return html;
}

QString DownloadThread::bioSearchIAFD(QString html, QString key){
    QString captureGroup("");
    QRegularExpression rx(".*<p class=\"bioheading\">"+key+"</p>\\s*<p class=\"biodata\">([^\\<]+)</p>.*");
    QRegularExpressionMatch m = rx.match(html);
    if (m.hasMatch()){  captureGroup = m.captured(1).trimmed();   }
    return captureGroup;
}

QMap<QString,QString> getFreeonesTags(QString html){
    QMap<QString,QString>tagMap;
    QRegularExpression rx("<dt>([^\\<]+)</dt>\\s*<dd>([^\\<]+)</dd>");
    QRegularExpressionMatchIterator it = rx.globalMatch(html);
    while(it.hasNext()){
        QRegularExpressionMatch match = it.next();
        QString key = match.captured(1);
        QString value = match.captured(2);
        if (!key.isEmpty() && !value.isEmpty()){
            qDebug("Key: %s, Value: %s", qPrintable(key), qPrintable(value));
            tagMap.insert(key, value);
        }
    }
    return tagMap;
}

QString DownloadThread::bioSearchFO(QString html, QString key){
    QString value("");
    QRegularExpression rx(".*<dt>"+key+"</dt>[\\s\\r\\n\\t]*<dd>(.+)</dd>.*");
    QRegularExpressionMatch match = rx.match(html);
    if (match.hasMatch() && match.captured(1) != "Unknown"){
        value = match.captured(1);
    }
    return value;
}

/** \brief Get Biographical Data from freeones.ca and set the passed bio data accordingly
 *  \param QString name:    Name of performer to get data about
 *  \param Biography *bio:  Pointer to Biography Data Structure into which the Information will be placed
 *  \return bool success:   True if the curl Request was successful, and it seems as though the data was retrieved.
 */
bool DownloadThread::getFreeonesData(QString name, Biography *bio){
    QString html = getHTML(Freeones, name);
    if (html.isEmpty()){
        qWarning("Freeones Returned Empty HTML for '%s'", qPrintable(name));
        return false;
    } else {
        getFreeonesTags(html);
        /*
        QFile file(QString("./%1 Freeones HTML.html").arg(name));
        if (file.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)){
            QTextStream out(&file);
            out << html;
            file.close();
        }
        */
    }

    if (!bio->has("measurements"))  {   bio->setMeasurements(bioSearchFO(html, "Measurements:"));    }
    if (!bio->has("aliases"))       {   bio->setAliases(bioSearchFO(html, "Aliases:"));              }
    if (!bio->has("city"))          {   bio->setCity(bioSearchFO(html, "Place of Birth:"));           }
    if (!bio->has("nationality"))   {   bio->setNationality(bioSearchFO(html, "Country of Origin:"));}
    if (!bio->has("eyes"))          {   bio->setEyeColor(bioSearchFO(html, "Eye Color:"));           }
    if (!bio->has("hair"))          {   bio->setHairColor(bioSearchFO(html, "Hair Color:"));         }
    if (!bio->has("piercings"))     {   bio->setPiercings(bioSearchFO(html, "Piercings:"));          }
    if (!bio->has("tattoos"))       {   bio->setTattoos(bioSearchFO(html, "Tattoos:"));              }
    if (!bio->has("birthdate"))     {
        QString birthdayElement     = bioSearchFO(html, "Date of Birth:");
        QRegularExpression birthdayRegex("([A-Za-z]+\\s\\d+,\\s\\d+)\\s*.*");
        QRegularExpressionMatch birthdayMatch = birthdayRegex.match(birthdayElement);
        if (birthdayMatch.hasMatch()){
            QString birthdayText = birthdayMatch.captured(1);
            QDate birthday = QDate::fromString(birthdayText, "MMMM d, yyyy");
            if (birthday.isValid()){
                bio->setBirthday(birthday);
            }
        }
    }
    QString active  = bioSearchFO(html, "Career Status");
    if (active.contains("Active")){
        bio->retired = false;
    }
    qDebug("Freeones Request for '%s' resulted in Bio of size %d", qPrintable(name), bio->size());
    return true;
}

Biography DownloadThread::freeones(QString name){
    Biography bio(name);
    getFreeonesData(name, &bio);
    return bio;
}


/** \brief Get Biographical Data from IAFD.com and set the passed bio data accordingly
 *  \param QString name:    Name of performer to get data about
 *  \param Biography *bio:  Pointer to Biography Data Structure into which the Information will be placed
 *  \return bool success:   True if the curl Request was successful, and it seems as though the data was retrieved.
 */
bool DownloadThread::getIAFDData(QString name, Biography *bio){
    QString html = getHTML(IAFD, name);
    // Verify that the html string isn't empty and contains some expected text.
    if (html.isEmpty()){
        qWarning("IAFD returned empty HTML for '%s'", qPrintable(name));
        return false;
    } else {
        /*
        QFile file(QString("./%1 IAFD HTML.html").arg(name));
        if (file.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)){
            QTextStream out(&file);
            out << html;
            file.close();
        }
        */
    }
    // PARSE OUT THE VALUES OF INTEREST
    bio->setName(name);
    if (!bio->has("height"))      {   bio->setHeight(Height::fromText(bioSearchIAFD(html, "Height")));  }
    if (!bio->has("ethnicity"))   {   bio->setEthnicity(bioSearchIAFD(html, "Ethnicity"));              }
    if (!bio->has("measurements")){   bio->setMeasurements(bioSearchIAFD(html, "Measurements"));        }
    if (!bio->has("tattoos"))     {   bio->setTattoos(bioSearchIAFD(html, "Tattoos"));                  }
    if (!bio->has("piercings"))   {   bio->setPiercings(bioSearchIAFD(html, "Piercings"));              }
    if (!bio->has("hair"))        {   bio->setHairColor(bioSearchIAFD(html, "Hair Color"));             }
    if (!bio->has("nationality")) {   bio->setNationality(bioSearchIAFD(html, "Nationality"));          }
    //if (!bio->has("city"))        {   bio->setCity(bioSearchIAFD(html, "Birthplace"));                  }
    // SPECIALIZED PARSING
    if (!bio->birthdate.isValid() || bio->birthdate.isNull()){
        QString bday    = bioSearchIAFD(html, "Birthday");
        if (!bday.isEmpty() && !bday.isNull()){
            bio->birthdate   = QDate::fromString(bday, "MMMM d, yyyy");
        }
    }
    if (bio->getWeight() <= 0){
        QString weightString = bioSearchIAFD(html, "Weight");
        qDebug("Weight String: '%s'", qPrintable(weightString));
        QRegularExpression rx("[^0-9]*([0-9]+)\\s*lbs.*");
        QRegularExpressionMatch m = rx.match(weightString);
        out();
        if (m.hasMatch()){
            try{
                bool ok = false;
                int weight = m.captured(1).toInt(&ok);
                qDebug("Converted Weight: %d", weight);
                if (ok){
                    bio->setWeight(weight);
                }
            } catch (std::exception &e){
                qWarning("Exception Caught while attempting to process weight");
            }
        }
    }
    out();
    qDebug("Retrieving Height");
    if (!bio->getHeight().nonZero()){
        QString heightStr = bioSearchIAFD(html, "Height");
        qDebug("Height String: '%s'", qPrintable(heightStr));
        QRegularExpression rx("([0-6]) feet, ([0-9]{1,2}) inches.*");
        QRegularExpressionMatch m = rx.match(heightStr);
        if (m.hasMatch()){
            int feet = m.captured(1).toInt();
            int inches = m.captured(2).toInt();
            qDebug("Height: %d' %d''", feet, inches);
            bio->setHeight(feet, inches);
        }
    }
    QRegularExpressionMatch careerMatch = QRegularExpression("class=\"biodata\">([0-9]{4})-?([0-9]{4})? \\(Started").match(html);
    if (careerMatch.hasMatch()){
        if (bio->careerStart.isNull() || !bio->careerStart.isValid()){
            bio->careerStart = QDate(careerMatch.captured(1).toInt(), 0, 0);
        }
        if ((careerMatch.lastCapturedIndex() > 1) && (bio->careerEnd.isNull() || !bio->careerEnd.isValid())){
            bio->careerEnd   = QDate(careerMatch.captured(2).toInt(), 0, 0);
            bio->retired = true;
        }
    }
    qDebug("IAFD Request for '%s' resulted in a bio of size %d", qPrintable(name), bio->size());
    return true;
}

Biography DownloadThread::iafd(QString name){
    Biography bio(name);
    getIAFDData(name, &bio);
    return bio;
}

void curlTool::receiveActor(ActorPtr a){
    if (a.isNull()){
        qWarning("Received Null Actor");
    } else {
        qDebug("Received Actor '%s'", qPrintable(a->getName()));
        this->actorList.push_back(a);
        this->additions++;
    }
}
void curlTool::downloadThreadComplete(){
    this->threadsFinished++;
    qDebug("%d/%d Threads Finished", this->threadsFinished, this->threadsStarted);
    emit updateProgress(threadsFinished);
    if (this->threadsFinished == this->threadsStarted){
        qDebug("*****************************\nAll Threads Finished\n*****************************");
        emit closeProgress(QString("Update Complete. %1 Names Searched, %2 Actor Objects Successfully Created (%3 items in list).").arg(nameList.size()).arg(additions).arg(actorList.size()));
        emit updateFinished(actorList);
    }
}

void curlTool::makeNewActors(QStringList nameList){
    resetCounters();
    this->actorList.clear();
    this->nameList = nameList;
    emit startProgress(QString("Updating the bios for %1 Actors").arg(nameList.size()), nameList.size());
    for (int i = 0; i < nameList.size(); ++i){
        QString name = nameList.at(i);
        DownloadThread *d = new DownloadThread(name);
        connect(d, SIGNAL(sendActor(ActorPtr)), this, SLOT(receiveActor(ActorPtr)));
        connect(d, SIGNAL(finished()),         this, SLOT(downloadThreadComplete()));
        this->threadPool.globalInstance()->start(d);
        this->threadsStarted++;
    }
    qDebug("*****************************\nAll Threads Started\n*****************************");
}

void curlTool::updateBio(ActorPtr a){
    this->currentActor = a;
    QString name = currentActor->getName();
    if (!name.isEmpty()){
        Biography b(name);
        DownloadThread::getFreeonesData(name, &b);
        qDebug("Freeones profile returned a bio of size %d", b.size());
        bool iafdProfileExists = DownloadThread::getIAFDData(name, &b);
        qDebug("IAFD profile returned a bio of size %d", b.size());
        QString headshot("");
        if (iafdProfileExists){
            headshot = downloadHeadshot(name);
        }
        this->currentActor = ActorPtr(new Actor(name, b, headshot));
    }
    qDebug("Returning Bio for %s", qPrintable(name));
    emit updateSingleProfile(currentActor);
}

void curlTool::resetCounters(){
    this->threadsFinished = 0;
    this->threadsStarted = 0;
    this->index = 0;
    this->actorList.clear();
    this->additions = 0;
}

/****************************************************************
 *                           PHOTOS                             *
 ****************************************************************/
///SLOT
void curlTool::downloadPhoto(ActorPtr a){
    bool success = false;
    QString photo = downloadHeadshot(a->getName());
    if (!photo.isEmpty() && QFileInfo(photo).exists()){
        success = true;
        a->setHeadshot(FilePath(photo));
    }
    emit finishedProcessing(success);
}
///SLOT


/** \brief  Download an image from the url provided, and save it to the path provided.
 *  \param  QString URL:            Web address to the link to save.
 *  \param  QString destination:    Absolute path to the location on disk where the link should be saved to.
 *  \return bool success:           true if download succeeded, false if it failed.
 */
bool DownloadThread::wget(QString url, QString filePath){
    bool success = false;
    QString cmd = QString("/usr/local/bin/wget %1 -O %2").arg(url).arg(filePath);
    if (!system_call(cmd).isEmpty()){
        success = QFileInfo(filePath).exists();
    }
    return success;
}

/** \brief  Download a headshot for the actor with the name passed to the function. Return empty string if download failed, otherwise,
 *          return the full path to the file.
 *  \param  QString actorName: Name of actor.
 *  \return QString fullPath:  Full path to the downloaded file. Empty if download failed.
 */
QString downloadHeadshot(QString actorName){
    QString returnValue("");
    if (headshotDownloaded(actorName)){
        qDebug("Headshot for '%s' already downloaded", qPrintable(actorName));
        returnValue = getHeadshotName(actorName);
    } else {
        QString html = DownloadThread::getHTML(IAFD, actorName);
        if (!html.isEmpty()){
            QRegularExpression rx(".*\\\"(http://www.iafd.com/graphics/headshots/.+\\.jpg)\\\">.*");
            QRegularExpressionMatch m = rx.match(html);
            if (m.hasMatch()){
                QString link = m.captured(1);
                QString fullpath = getHeadshotName(actorName);
                if (DownloadThread::wget(link, fullpath)){
                    returnValue = fullpath;
                    qDebug("Got Headshot for '%s'", qPrintable(actorName));
                } else {
                    qWarning("Failed to Download Headshot for '%s'", qPrintable(actorName));
                }
            } else {
                qWarning("No Headshot Found in IAFD For %s", qPrintable(actorName));
            }
        } else {
            qWarning("No IAFD Page Found for %s", qPrintable(actorName));
        }
    }
    return returnValue;
}

//----------------------------------------------------------------
//		THUMBNAILS
//----------------------------------------------------------------

/** \brief Generate Thumbnails for the video file that the Scene passed refers to. */
#warning Incomplete function for thumbnail generation.
bool curlTool::generateThumbnail(ScenePtr s){
    if (s->exists()){
        /*
        QString cmd = QString("ffmpg -i %1 -vf fps=%2 scale=\'min(%3, iw):-1\' %4").arg(file.unixSafe()).arg()
        std::string command("ffmpeg -i " + file.unixSafe() + " -vf fps=" + THUMBNAIL_RATE + " scale=\'min(" + THUMBNAIL_MAX_SIZE + "\\, iw):-1\' " + destination);
        shell_it(command);
        */
    }
    return false;
}
