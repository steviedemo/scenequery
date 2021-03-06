#include "config.h"
#include "curlTool.h"
#include "filenames.h"
#include "Actor.h"
#include "Scene.h"
#include "genericfunctions.h"
#include <curl/curl.h>
#include <stdio.h>
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
#include <QProcess>
#define out();  qDebug("%s::%s::%d", __FILE__,__FUNCTION__,__LINE__);
#define FREEONES_TAG_REGEX  "<dt>([^\\<]+)[:]?</dt>\\s*<dd>([^\\<]+)</dd>"
#define IAFD_TAG_REGEX      ".*<p class=\"bioheading\">([^\\<]+)</p>\\s*<p class=\"biodata\">([^\\<]+)</p>.*"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

QString request(QString urlString){
    QString html("");
    try{
        CURL *curl;
        //CURLcode res;
        std::string readBuffer;
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, qPrintable(urlString));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK){
            qWarning("Error: Curl request returned error code '%d'", res);
        }
        curl_easy_cleanup(curl);
        html = QString::fromStdString(readBuffer);
    } catch(std::exception &e){
        qWarning("Caught Exception while making curl Request to '%s': %s", qPrintable(urlString), e.what());
    }

    return html;
}

curlTool::curlTool(){
    this->dataPath = findDataLocation();
    this->headshotPath = findHeadshotLocation();
    this->thumbnailPath = findThumbnailLocation();
}

DownloadThread::DownloadThread(QString name): name(name), html(""),photo(""){
    this->actor = QSharedPointer<Actor>(new Actor(name));
    task = Curl::MAKE_BIO;
}
DownloadThread::DownloadThread(ActorPtr a):name(a->getName()), html(""), photo(""), actor(a){
    task = Curl::UPDATE_BIO;
}

curlTool::~curlTool(){}
DownloadThread::~DownloadThread(){}

/** \brief DownloadThread's Main Routine */
void DownloadThread::run(){
    if (!name.isEmpty()){
        Biography b(name);
        if (task == Curl::UPDATE_BIO){
            b.copy(actor->getBio());
        }
        this->actor = curlTool::downloadActor(name, b);
        emit sendActor(actor);
    }
    emit finished();
}

QString curlTool::getHTML(Website w, QString name){
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
        html = request(url);
    } else if (w == IAFD) {
        output << "www.iafd.com/person.rme/perfid=" << nameCopy_1.toLower().remove(QRegularExpression("[\\s\\.\\-\\']")) << "/gender=f/" << nameCopy_2.toLower().replace(" ", "-") << ".htm";
        if (!system_call_blocking("/usr/local/bin/curl", QStringList() << url, html)){
            qWarning("Error Getting HTML from IAFD for %s", qPrintable(name));
        }
    }
    return html;
}

QString curlTool::bioSearchIAFD(QString html, QString key){
    QString captureGroup("");
    QRegularExpression rx(".*<p class=\"bioheading\">"+key+"</p>\\s*<p class=\"biodata\">([^\\<]+)</p>.*");
    QRegularExpressionMatch m = rx.match(html);
    if (m.hasMatch()){  captureGroup = m.captured(1).trimmed();   }
    return captureGroup;
}

QMap<QString,QString> curlTool::parseBioTags(QString html, Website site){
    QMap<QString,QString>tagMap;
    QRegularExpression rx;
    if (site == Freeones){
        rx.setPattern(FREEONES_TAG_REGEX);
    } else {
        rx.setPattern(IAFD_TAG_REGEX);
    }
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


QString curlTool::bioSearchFO(QString html, QString key){
    QString value("");
    QRegularExpression rx(".*<dt>"+key+":?</dt>\\s*<dd>([^\\<]+)</dd>.*");
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
bool curlTool::getFreeonesData(QString name, Biography &bio, QString &html){
    bool success = false;
    html = getHTML(Freeones, name);
    if (html.isEmpty()){
        qWarning("Freeones Returned Empty HTML for '%s'", qPrintable(name));
    } else {
        //parseBioTags(html, Freeones);
        if (bio.getMeasurements().isEmpty())  {   bio.setMeasurements(bioSearchFO(html, "Measurements"));       }
        if (bio.getAliases().isEmpty())       {   bio.setAliases(bioSearchFO(html, "Aliases"));                 }
        if (bio.getCity().isEmpty())          {   bio.setCity(bioSearchFO(html, "Place of Birth"));             }
        if (bio.getNationality().isEmpty())   {   bio.setNationality(bioSearchFO(html, "Country of Origin"));   }
        if (bio.getEyeColor().isEmpty())      {   bio.setEyeColor(bioSearchFO(html, "Eye Color"));              }
        if (bio.getHairColor().isEmpty())     {   bio.setHairColor(bioSearchFO(html, "Hair Color"));            }
        if (bio.getPiercings().isEmpty())     {   bio.setPiercings(bioSearchFO(html, "Piercings"));             }
        if (bio.getTattoos().isEmpty())       {   bio.setTattoos(bioSearchFO(html, "Tattoos"));                 }
        if (!bio.has("birthdate"))     {
            QString birthdayElement     = bioSearchFO(html, "Date of Birth");
            QRegularExpression birthdayRegex("([A-Za-z]+\\s\\d+,\\s\\d+)\\s*.*");
            QRegularExpressionMatch birthdayMatch = birthdayRegex.match(birthdayElement);
            if (birthdayMatch.hasMatch()){
                QString birthdayText = birthdayMatch.captured(1);
                QDate birthday = QDate::fromString(birthdayText, "MMMM d, yyyy");
                if (birthday.isValid()){
                    bio.setBirthday(birthday);
                }
            }
        }
        QString active  = bioSearchFO(html, "Career Status");
        if (active.contains("Active")){
            bio.retired = false;
        }
        success = true;
   //     qDebug("Freeones Request for '%s' resulted in Bio of size %d", qPrintable(name), bio.size());
    }
    return success;
}

bool curlTool::getFreeonesData(QString name, Biography &bio){
    QString html("");
    return getFreeonesData(name, bio, html);
}

/** \brief Get Biographical Data from IAFD.com and set the passed bio data accordingly
 *  \param QString name:    Name of performer to get data about
 *  \param Biography &bio:  Reference to Biography Data Structure into which the Information will be placed
 *  \param QString &html:   Reference to QString where HTML of actor's profile page will be placed.
 *  \return bool success:   True if the curl Request was successful, and it seems as though the data was retrieved.
 */
bool curlTool::getIAFDData(QString name, Biography &bio, QString &html){
    html = getHTML(IAFD, name);
    bool success = false;
    // Verify that the html string isn't empty and contains some expected text.
    if ((success = !html.isEmpty())){
       //parseBioTags(html, IAFD);
        // PARSE OUT THE VALUES OF INTEREST
        bio.setName(name);
        if (!bio.has("height"))      {   bio.setHeight(Height::fromText(bioSearchIAFD(html, "Height")));  }
        if (bio.getEthnicity().isEmpty())       {   bio.setEthnicity(bioSearchIAFD(html, "Ethnicity"));              }
        if (bio.getMeasurements().isEmpty())    {   bio.setMeasurements(bioSearchIAFD(html, "Measurements"));        }
        if (bio.getTattoos().isEmpty())         {   bio.setTattoos(bioSearchIAFD(html, "Tattoos"));                  }
        if (bio.getPiercings().isEmpty())       {   bio.setPiercings(bioSearchIAFD(html, "Piercings"));              }
        if (bio.getHairColor().isEmpty())       {   bio.setHairColor(bioSearchIAFD(html, "Hair Color"));             }
        if (bio.getNationality().isEmpty())     {   bio.setNationality(bioSearchIAFD(html, "Nationality"));          }
        // SPECIALIZED PARSING
        if (!bio.birthdate.isValid() || bio.birthdate.isNull()){
            QString bday    = bioSearchIAFD(html, "Birthday");
            if (!bday.isEmpty() && !bday.isNull()){
                bio.birthdate   = QDate::fromString(bday, "MMMM d, yyyy");
            }
        }
        if (bio.getWeight() <= 0){
            QString weightString = bioSearchIAFD(html, "Weight");
            QRegularExpression rx("[^0-9]*([0-9]+)\\s*lbs.*");
            QRegularExpressionMatch m = rx.match(weightString);
            if (m.hasMatch()){
                try{
                    bool ok = false;
                    int weight = m.captured(1).toInt(&ok);
                    if (ok){
                        bio.setWeight(weight);
                    }
                } catch (std::exception &e){
                    qWarning("Exception Caught while attempting to process weight");
                }
            }
        }
        if (!bio.getHeight().nonZero()){
            QString heightStr = bioSearchIAFD(html, "Height");
            QRegularExpression rx("([0-6]) feet, ([0-9]{1,2}) inches.*");
            QRegularExpressionMatch m = rx.match(heightStr);
            if (m.hasMatch()){
                int feet = m.captured(1).toInt();
                int inches = m.captured(2).toInt();
                bio.setHeight(feet, inches);
            }
        }
        QRegularExpressionMatch careerMatch = QRegularExpression("class=\"biodata\">([0-9]{4})-?([0-9]{4})? \\(Started").match(html);
        if (careerMatch.hasMatch()){
            if (bio.careerStart.isNull() || !bio.careerStart.isValid()){
                bio.careerStart = QDate(careerMatch.captured(1).toInt(), 0, 0);
            }
            if ((careerMatch.lastCapturedIndex() > 1) && (bio.careerEnd.isNull() || !bio.careerEnd.isValid())){
                bio.careerEnd   = QDate(careerMatch.captured(2).toInt(), 0, 0);
                bio.retired = true;
            }
        }
    } else {
        qWarning("IAFD returned empty HTML for '%s'", qPrintable(name));
    }
  //  qDebug("IAFD Request for '%s' resulted in a bio of size %d", qPrintable(name), bio.size());
    return success;
}
bool curlTool::getIAFDData(QString name, Biography &bio){
    QString html("");
    return getIAFDData(name, bio, html);
}

void curlTool::receiveActor(ActorPtr a){
    if (a.isNull()){
        qWarning("Received Null Actor");
    } else {
     //   qDebug("Received Actor '%s'", qPrintable(a->getName()));
        qDebug("Curl Thread received '%s' from the download thread, with a profile of size %d", qPrintable(a->getName()), a->size());
        curlMx.lock();
        this->actorList.push_back(a);
        this->additions++;
        curlMx.unlock();
    }
    curlMx.lock();
    emit updateProgress(++index);
    curlMx.unlock();
}

void curlTool::downloadThreadComplete(){
    this->threadsFinished++;
    qDebug("%d/%d Threads Finished", this->threadsFinished, this->threadsStarted);
    emit updateProgress(threadsFinished);
    if (this->threadsFinished == this->threadsStarted){
     //   qDebug("*****************************\nAll Threads Finished\n*****************************");
        emit closeProgress(QString("Update Complete. %1 Names Searched, %2 Actor Objects Successfully Created (%3 items in list).").arg(nameList.size()).arg(additions).arg(actorList.size()));
        if (this->currentTask == Curl::SCANNING_BIOS){
            emit ct_to_db_storeActors(actorList);
        } else {
            emit updateFinished(actorList);
        }
    }
}

void curlTool::updateBios(ActorList list){
    this->currentTask = Curl::UPDATE_BIO;
    QMutexLocker ml(&threadMx);
    resetCounters();
    if (list.size() > 0){
        this->actorList.clear();
        this->nameList.clear();
        this->threadPool = new QThreadPool(this);
        emit startProgress(QString("Updating the Bios for %1 Actors").arg(list.size()), list.size());
        for (int i = 0; i < list.size(); ++i){
            ActorPtr a = list.at(i);
            DownloadThread *d = new DownloadThread(a);
            connect(d, SIGNAL(sendActor(ActorPtr)), this, SLOT(receiveActor(ActorPtr)));
            connect(d, SIGNAL(finished()),          this, SLOT(downloadThreadComplete()));
            this->threadPool->globalInstance()->start(d);
            this->threadsStarted++;
        }
        qDebug("Waiting for %d Curl Threads to finish", list.size());
        threadPool->waitForDone(180000); // 3 Minute Timeout
        delete threadPool;
    }
}

void curlTool::makeNewActors(QStringList list){
    this->currentTask = Curl::MAKE_BIO;
    QMutexLocker ml(&threadMx);
    resetCounters();
    this->actorList.clear();
    this->nameList = list;
    if (nameList.size() > 0){
        this->threadPool = new QThreadPool(this);
        emit startProgress(QString("Updating the bios for %1 Actors").arg(nameList.size()), nameList.size());
        for (int i = 0; i < nameList.size(); ++i){
            QString name = nameList.at(i);
            DownloadThread *d = new DownloadThread(name);
            connect(d, SIGNAL(sendActor(ActorPtr)), this, SLOT(receiveActor(ActorPtr)));
            connect(d, SIGNAL(finished()),         this, SLOT(downloadThreadComplete()));
            this->threadPool->globalInstance()->start(d);
            this->threadsStarted++;
        }
        qDebug("Waiting for %d Curl threads to finish", nameList.size());
        this->threadPool->waitForDone();
        delete threadPool;
    }
}

void curlTool::db_to_ct_buildActors(QStringList names){
    this->currentTask = Curl::SCANNING_BIOS;
    QMutexLocker ml (&threadMx);
    qDebug("Curl Thread Received List of %d names from the File Scanner.", names.size());
    resetCounters();
    this->actorList = {};
    this->nameList = names;
    if (nameList.size() > 0){
        this->threadPool = new QThreadPool(this);
        emit startProgress(QString("Building Profiles for %1 Actors").arg(names.size()), names.size());
        this->threadsStarted = names.size();
        foreach(QString name, nameList){
            DownloadThread *d = new DownloadThread(name);
            connect(d, SIGNAL(sendActor(ActorPtr)), this, SLOT(receiveActor(ActorPtr)));
            connect(d, SIGNAL(finished()), this, SLOT(downloadThreadComplete()));
            this->threadPool->globalInstance()->start(d);
        }
        qDebug("Waiting for %d Curl Threads to finish", nameList.size());
        this->threadPool->waitForDone();
        delete threadPool;
    }
}

ActorPtr curlTool::downloadActor(QString name, Biography bio) {
    ActorPtr actor = ActorPtr(0);
    if (!name.isEmpty()){
        QString html("");
        getFreeonesData(name, bio);
        if (getIAFDData(name, bio, html)){
            qDebug("%s got bio of size %d", qPrintable(name), bio.size());
        }
        actor = ActorPtr(new Actor(name, bio, ""));
        if (getProfilePhoto(name) == DEFAULT_PROFILE_PHOTO && !html.isEmpty()){
            downloadHeadshot(actor, html);
            actor->setHeadshot(getProfilePhoto(name));
        }
    }
    return actor;
}

void curlTool::pd_to_ct_getActor(QString name){
    Biography bio(name);
    this->currentActor = downloadActor(name, bio);
    emit ct_to_pd_sendActor(currentActor);
}

void curlTool::apv_to_ct_getProfile(QString name){
    if (!name.isEmpty()){
        qDebug("Updating actor '%s'", qPrintable(name));
        Biography bio(name);
        this->currentActor = downloadActor(name, bio);
        if (!currentActor.isNull()){
            emit ct_to_apv_sendActor(currentActor);
        } else {
            qWarning("Got Null Profile.");
        }
    } else {
        qWarning("Can't update actor with empty name");
    }
}

void curlTool::updateBio(ActorPtr a){
    this->currentActor = a;
    QString name = currentActor->getName();
    if (!name.isEmpty()){
        Biography bio(a->getBio());
        this->currentActor = downloadActor(name, bio);
    }
    //qDebug("Returning Bio for %s", qPrintable(name));
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
    QString photo = downloadHeadshot(a->getName());
    if (!photo.isEmpty() && QFileInfo(photo).exists()){
        a->setHeadshot(photo);
    }
    emit updateSingleProfile(a);
}
///SLOT


/** \brief  Download an image from the url provided, and save it to the path provided.
 *  \param  QString URL:            Web address to the link to save.
 *  \param  QString destination:    Absolute path to the location on disk where the link should be saved to.
 *  \return bool success:           true if download succeeded, false if it failed.
 */
bool curlTool::wget(QString url, QString filePath){
    bool success = false;
    QStringList args;
    args << url << "-O" << filePath << ">" << "/dev/null";
    QProcess *process = new QProcess();
    QString command = "/usr/local/bin/wget";
    process->start(command, args);
    if (!process->waitForStarted()){
        qWarning("Error Starting wget QProcess for %s", qPrintable(filePath));
    } else if (!process->waitForFinished()){
        qWarning("QProcess Timed out while waiting for wget");
    } else {
        //qDebug("wget got profile photo %s", qPrintable(filePath));
        success = QFileInfo(filePath).exists();
    }
    delete process;
    /*
    QString cmd = QString("/usr/local/bin/wget %1 -O %2").arg(url).arg(filePath);
    if (!system_call(cmd).isEmpty()){
        success = QFileInfo(filePath).exists();
    }
    */
    return success;
}

/** \brief  Download a headshot for the actor with the name passed to the function. Return empty string if download failed, otherwise,
 *          return the full path to the file.
 *  \param  QString actorName: Name of actor.
 *  \return QString fullPath:  Full path to the downloaded file. Empty if download failed.
 */
QString curlTool::downloadHeadshot(QString actorName){
    QString returnValue("");
    QString html = curlTool::getHTML(IAFD, actorName);
    if (!html.isEmpty()){
        QRegularExpression rx(".*\\\"(http://www.iafd.com/graphics/headshots/.+\\.jpg)\\\">.*");
        QRegularExpressionMatch m = rx.match(html);
        if (m.hasMatch()){
            QString link = m.captured(1);
            QString fullpath = getHeadshotName(actorName);
            if (wget(link, fullpath)){
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
    return returnValue;
}
QString curlTool::getHeadshotLink(QString html){
    QString link("");
    QRegularExpression rx(".*\\\"(http://www.iafd.com/graphics/headshots/.+\\.jpg)\\\">.*");
    QRegularExpressionMatch m = rx.match(html);
    if (m.hasMatch()){
        link = m.captured(1);
    } else {
        link = "";
    }
    return link;
}

bool curlTool::downloadHeadshot(ActorPtr a, QString html){
    bool success = false;
    if (!html.isEmpty()){
        QString link = getHeadshotLink(html);
        if (link.isEmpty()){
            qWarning("Unable to find headshot link for %s", qPrintable(a->getName()));
        } else {
            QString photoPath = getHeadshotName(a->getName());
            if ((success = wget(link, photoPath))){
                a->setHeadshot(photoPath);
            }
        }
    }
    if (!success){
        a->setHeadshot(DEFAULT_PROFILE_PHOTO);
    }
    return success;
}
