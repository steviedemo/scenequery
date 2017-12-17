#include "curlTool.h"
#include "qtcurl/QtCUrl.h"
#include "Actor.h"
#include "genericfunctions.h"
#include <curl/curl.h>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>
#include <QVector>
curlTool::curlTool(){
    this->datalocation = "data";//QStandardPaths::AppLocalDataLocation();
    this->headshotPath = QString("%1/%2").arg(APP_DATA_PATH).arg(HEADSHOT_PATH);
//    if (!QDir(datalocation).exists()){
//    #warning Change '.' to whichever directory you want to store data in.
//        if (!QDir::current().mkdir(datalocation)){
//            qWarning("Unable to create data path in %s", qPrintable(datalocation));
//        }
//    }

}
curlTool::~curlTool(){
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}
QString curlTool::request(QString url){
    QString html("");
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    if (curl){
        curl_easy_setopt(curl, CURLOPT_URL, qPrintable(url));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    html = QString::fromStdString(readBuffer);
    return html;
}

QString curlTool::getHTML(Website w, QString name){
    QString html("");
    QString lc = name.toLower();
    if (w == IAFD){
        QString formattedName = lc.replace(QChar(' '), QChar('_')).remove(QChar('.'));
        QString url = QString("%1/%2_links/%3").arg(FREEONES_URL).arg(lc.at(0)).arg(formattedName);
        qDebug("Retrieving Freeones bio from: %s", qPrintable(url));
        QString html = request(url);
    } else if (w == Freeones) {
        QString url = QString("%1/perfid=%2/gender=f/%3.htm").arg(IAFD_URL).arg(lc.remove(' ')).arg(lc.replace(' ', '-'));
        qDebug("Retrieving IAFD Bio from: %s", qPrintable(url));
        html = system_call(QString("curl %1").arg(url));
    }
    return html;
}

QString curlTool::bioSearchIAFD(QString html, QString key){
    QString captureGroup("");
    QRegularExpression rx;
    rx.setPattern(QString("<p class=\"bioheading\">%1</p><p class=\"biodata\">([a-zA-Z0-9/()\\s]+)</p>").arg(key));
    QRegularExpressionMatch m = rx.match(html);
    if (m.hasMatch()){  captureGroup = m.captured(1);   }
    return captureGroup;
}

QString curlTool::bioSearchFO(QString html, QString key){
    QString value("");
    QRegularExpression rx;
    rx.setPattern(QString("<dt>%1:\\s?</dt>\n<dd>([^\n]+)</dd>").arg(key));
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
bool curlTool::getFreeonesData(QString name, Biography *bio){
    QString html = getHTML(Freeones, name);
    if (html.isEmpty()){
        qWarning("Retrieved Empty HTML from call to Freeones for '%s'", qPrintable(name));
        return false;
    }
    html = html.right(html.size() - html.indexOf("<dt>Babe Name:"));
    if (html.contains("</div>")){
        html.chop(html.size() - html.indexOf("</div>"));
    } else {
        qWarning("Unable to Find ending </div> tag on freeones page for '%s'", qPrintable(name));
        return false;
    }
    if (bio->measurements.isEmpty()){   bio->measurements= bioSearchFO(html, "Measurements");       }
    if (bio->aliases.isEmpty())     {   bio->aliases     = bioSearchFO(html, "Aliases");            }
    if (bio->city.isEmpty())        {   bio->city        = bioSearchFO(html, "Place of Birth");     }
    if (bio->nationality.isEmpty()) {   bio->nationality = bioSearchFO(html, "Country of Origin");  }
    if (bio->eyes.isEmpty())        {   bio->eyes        = bioSearchFO(html, "Eye Color");          }
    if (bio->hair.isEmpty())        {   bio->hair        = bioSearchFO(html, "Hair Color");         }
    if (bio->piercings.isEmpty())   {   bio->piercings   = bioSearchFO(html, "Piercings");          }
    if (bio->tattoos.isEmpty())     {   bio->tattoos     = bioSearchFO(html, "Tattoos");            }
    if (!bio->birthdate.isValid() || bio->birthdate.isNull()){
        QString bday     = bioSearchFO(html, "Date of Birth");
        if (!bday.isEmpty() && !bday.isNull()){
            bio->birthdate = QDate::fromString(bday, "MMMM d, yyyy");
        }
    }
    QString active  = bioSearchFO(html, "Career Status");
    if (active.contains("Active")){
        bio->retired = false;
    }
    return true;
}

Biography curlTool::freeones(QString name){
    Biography bio;
    bio.name = name;
    getFreeonesData(name, &bio);
    return bio;
}

/** \brief Get Biographical Data from IAFD.com and set the passed bio data accordingly
 *  \param QString name:    Name of performer to get data about
 *  \param Biography *bio:  Pointer to Biography Data Structure into which the Information will be placed
 *  \return bool success:   True if the curl Request was successful, and it seems as though the data was retrieved.
 */
bool curlTool::getIAFDData(QString name, Biography *bio){
    QString html = getHTML(IAFD, name);
    // Verify that the html string isn't empty and contains some expected text.
    if (html.isEmpty()){
        qWarning("Got empty HTML From IAFD curl request for '%s'", qPrintable(name));
        return false;
    }
    if (!html.contains("<p><b>Find where <a style=")){
        qWarning("Couldn't find expected section ending in HTML for '%s's IAFD page.", qPrintable(name));
        return false;
    }
    // TRIM OUT IRRELEVANT HTML SECTIONS
    int sectionStart = html.indexOf("<p class=\"bioheading\">Ethnicity");
    html.right(html.size() - sectionStart); // Remove the first chunk of the text.
    int sectionEnd = html.indexOf("<p><b>Find where <a style=");
    html.left(sectionEnd);                  // Isolate the remaining first half that is of interest

    // PARSE OUT THE VALUES OF INTEREST
    if (!bio->height.isValid())     {   bio->height         = Height::fromText(bioSearchIAFD(html, "Height"));  }
    if (bio->ethnicity.isEmpty())   {   bio->ethnicity      = bioSearchIAFD(html, "Ethnicity");                 }
    if (bio->measurements.isEmpty()){   bio->measurements   = bioSearchIAFD(html, "Measurements");              }
    if (bio->piercings.isEmpty())   {   bio->piercings      = bioSearchIAFD(html, "Tattoos");                   }
    if (bio->hair.isEmpty())        {   bio->hair           = bioSearchIAFD(html, "Hair Color");                }
    if (bio->nationality.isEmpty()) {   bio->nationality    = bioSearchIAFD(html, "Nationality");               }
    if (bio->city.isEmpty())        {   bio->city           = bioSearchIAFD(html, "Birthplace");                }
    // SPECIALIZED PARSING
    if (!bio->birthdate.isValid || bio->birthdate.isNull()){
        QString bday    = bioSearchIAFD(html, "Birthday");
        if (!bday.isEmpty() && !bday.isNull()){
            bio->birthdate   = QDate::fromString(bday, "MMMM d, yyyy");
        }
    }
    if (bio->weight == 0){
        QString weightString = bioSearchIAFD(html, "Weight");
        QRegularExpressionMatch weightMatch = QRegularExpression(".*([0-9]+)\\s*\\(.*\\).*").match(weightString);
        if (weightMatch.hasMatch()){
            bio->weight = weightMatch.captured(1).toInt();
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
    return true;
}

Biography curlTool::iafd(QString name){
    Biography bio;
    bio.name = name;
    getIAFDData(name, &bio);
    return bio;
}

/****************************************************************
 *                           PHOTOS                             *
 ****************************************************************/
QString curlTool::headshotDownloaded(QString name){
    QString location("");
    QString filepath = QString("%1/%2").arg(this->headshotPath).arg(headshot(name));
    if (QFileInfo(filepath).exists()){
        location = filepath;
    }
    return location;
}

QString headshotName(QString name){
    QString filename = name.toLower().replace(' ', '_').remove(QRegularExpression("[\\t\\n'.]"));
    filename.append(".jpeg");
    return filename;
}

bool curlTool::wget(QString url, QString destination){
    QString cmd = QString("wget -O %1 %2").arg(destination).arg(url);
    return (system(qPrintable(cmd)) == EXIT_SUCCESS);
}

bool curlTool::downloadHeadshot(QString name){
    bool success = false;
    if (headshotDownloaded(name)){
        qDebug("Headshot for '%s' already downloaded", qPrintable(name));
        success = true;
    } else {
        QString html = getHTML(IAFD, name);
        if (!html.isEmpty()){
            const QRegularExpression rx(".*<div id=\"headshot\">.*src=\"(.*)\">\\s*</div>\\s*<p class=\"headshotcaption\">.*");
            QRegularExpressionMatch m = rx.match(html);
            if (m.hasMatch()){
                QString link = m.captured(1);
                QString pathToFile = QString("%1/%2").arg(headshotPath).arg(headshotName(name));
                if (wget(link, pathToFile)){
                    if ((success = QFileInfo(pathToFile).exists())){
                        qDebug("Got Headshot for '%s'", qPrintable(name));
                    } else {
                        qWarning("wget request successful for '%s' headshot, but file not found after downloading.", qPrintable(name));
                    }
                } else {
                    qWarning("Failed to Download Headshot for '%s'", qPrintable(name));
                }
            } else {
                qWarning("No Headshot Found For %s", qPrintable(name));
            }
        }
    }
    return success;
}

//----------------------------------------------------------------
//		THUMBNAILS
//----------------------------------------------------------------
QString formatImageName(QString s, QString extension){
    QString n("");
    if (!s.isEmpty() && !s.isNull()){
        s.replace(QRegularExpression("[\\s,']"), "_");
        n = QString("%1.%2").arg(s.toLower()).arg(extension);
    }
    return n;
}

QString getThumbnailFormat(QString filename){
    return QString("%1/%2/%3").arg(DATAPATH).arg(THUMBNAIL_PATH).arg(toImageFormatString(filename));
}

bool thumbnailExists(Scene &s)
{
    FilePath thumbnail(THUMBNAIL_PATH, toImage(s.filename(), "png"));
    return thumbnail.exists();
}

void generateThumbnails(Scene &s)
{
    if (s.exists())
    {
        std::string destination = getThumbnailFormat(s.filename());
        std::string command("ffmpeg -i " + s.unix() + " -vf fps=" + THUMBNAIL_RATE + " scale=\'min(" + THUMBNAIL_MAX_SIZE + "\\, iw):-1\' " + destination);
        shell_it(command);
    }
}
void generateThumbnails(std::vector<Scene> &s)
{
    size_t idx = 0;
    omp_set_dynamic(0);
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (size_t i = 0; i < s.size(); ++i)
    {
        #pragma omp critical
        {
            std::cout << ++idx << "/" << s.size() << ":\t" << s.at(i).filename() << std::endl;
        }
        generateThumbnails(s.at(i));
    }
}
