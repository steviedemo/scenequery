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

Biography curlTool::freeones(QString name){
    Biography bio;
    QString html = getHTML(Freeones, name);
    if (html.isEmpty() || html.isNull()){
        return bio;
    }
    /**************TRIM OUT IRRELEVANT HTML SECTIONS****************/
    // Trim the beginning of the text
    html = html.right(html.size() - html.indexOf("<dt>Babe Name:"));
    // Trim the end of the text
    if (!html.contains("</div>"))
        return bio;
    html.chop(html.size() - html.indexOf("</div>"));
    /**************PARSE OUT THE VALUES OF INTEREST****************/
    bio.measurements= bioSearchFO(html, "Measurements");
    bio.aliases     = bioSearchFO(html, "Aliases");
    bio.city        = bioSearchFO(html, "Place of Birth");
    bio.nationality = bioSearchFO(html, "Country of Origin");
    bio.eyes        = bioSearchFO(html, "Eye Color");
    bio.hair        = bioSearchFO(html, "Hair Color");
    bio.piercings   = bioSearchFO(html, "Piercings");
    bio.tattoos     = bioSearchFO(html, "Tattoos");
    QString bday    = bioSearchFO(html, "Date of Birth");
    if (!bday.isEmpty() && !bday.isNull()){
        bio.birthdate = QDate::fromString(bday, "MMMM d, yyyy");
    }
    QString active  = bioSearchFO(html, "Career Status");
    if (active.contains("Active")){
        bio.retired = false;
    }
    return bio;
}

Biography curlTool::iafd(QString name){
    Biography bio;
    QString html = getHTML(IAFD, name);
    /**************TRIM OUT IRRELEVANT HTML SECTIONS****************/
    int sectionStart = html.indexOf("<p class=\"bioheading\">Ethnicity");
    html.right(html.size() - sectionStart); // Remove the first chunk of the text.
    int sectionEnd = html.indexOf("<p><b>Find where <a style=");
    html.left(sectionEnd);                  // Isolate the remaining first half that is of interest

    /**************PARSE OUT THE VALUES OF INTEREST****************/
    bio.ethnicity   = bioSearchIAFD(html, "Ethnicity");
    bio.height      = Height::fromText(bioSearchIAFD(html, "Height"));
    bio.measurements= bioSearchIAFD(html, "Measurements");
    bio.tattoos     = bioSearchIAFD(html, "Tattoos");
    bio.piercings   = bioSearchIAFD(html, "Piercings");
    bio.hair        = bioSearchIAFD(html, "Hair Color");
    bio.nationality = bioSearchIAFD(html, "Nationality");
    bio.city        = bioSearchIAFD(html, "Birthplace");
    /**********************SPECIALIZED PARSING*********************/
    QString bday    = bioSearchIAFD(html, "Birthday");
    if (!bday.isEmpty() && !bday.isNull()){
        bio.birthdate   = QDate::fromString(bday, "MMMM d, yyyy");
    }
    QString weightString = bioSearchIAFD(html, "Weight");
    QRegularExpressionMatch weightMatch = QRegularExpression(".*([0-9]+)\\s*\\(.*\\).*").match(weightString);
    if (weightMatch.hasMatch()){
        bio.weight = weightMatch.captured(1).toInt();
    }
    QRegularExpressionMatch careerMatch = QRegularExpression("class=\"biodata\">([0-9]{4})-?([0-9]{4})? \\(Started").match(html);
    if (careerMatch.hasMatch()){
        bio.careerStart = QDate(careerMatch.captured(1).toInt(), 0, 0);
        if (careerMatch.lastCapturedIndex() > 1){
            bio.careerEnd   = QDate(careerMatch.captured(2).toInt(), 0, 0);
            bio.retired = true;
        }
    }

    /***************************RETURN BIO***************************/
    return bio;
}
/****************************************************************
 *                           PHOTOS                             *
 ****************************************************************/
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
    QString html = getHTML(IAFD, name);
    if (!html.isEmpty()){
        const QRegularExpression rx(".*<div id=\"headshot\">.*src=\"(.*)\">\\s*</div>\\s*<p class=\"headshotcaption\">.*");
        QRegularExpressionMatch m = rx.match(html);
        if (m.hasMatch()){
            QString link = m.captured(1);
            QString destination = QString("%1/%2").arg(headshotPath).arg(headshotName(name));
            if (wget(link, destination)){
                success = true;
                qDebug("Got Headshot for '%s'", qPrintable(name));
            } else {
                qWarning("Failed to Download Headshot for '%s'", qPrintable(name));
            }
        } else {
            qWarning("No Headshot Found For %s", qPrintable(name));
        }
    }
    return success;
}
