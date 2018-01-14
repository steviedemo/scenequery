#include "genericfunctions.h"
#include <QStringList>
#include <QDir>
#include <QDebug>
#include <QImage>
#include <QProcess>
#include <QRegularExpression>
#include <QDate>
#include "Actor.h"
#include "SceneList.h"
#define AVG(a,b)    ( ((((a)^(b)) & 0xfefefefeUL) >> 1) + ((a)&(b)) )

bool nonzero(double d){
    return d > 0;
}
bool nonzero(int i){
    return (i > 0);
}
bool empty(QString s){
    return s.isEmpty();
}
bool full(QString s){
    return !(s.isEmpty());
}
bool valid(QDate d){
    return (d.isValid());
}
bool valid(QDateTime d){
    return (d.isValid());
}

QStringList getRatingList(){
    QStringList ratings;
    ratings << "A+++" << "A++" << "A+" << "A" << "A-" << "B+" << "B" << "B-" << "C" << "R";
    return ratings;
}

QImage scaleImage(QString file, int height, Qt::TransformationMode mode){
    QImage source(file);
    int width = ((height*source.width())/source.height());
    return source.scaled(width, height, Qt::KeepAspectRatio, mode);
}
/*
QImage scaleImage(QString file, int targetHeight){
    QImage source(file, "ARGB32");
    return scaleImage(source.convertToFormat(QImage::Format_ARGB32), targetHeight);
}
*/
QImage scaleImage(QImage source, int height){
    Q_ASSERT(source.format() == QImage::Format_ARGB32);
    int width = ((height*source.width())/source.height());
    QImage dest(width, height, QImage::Format_ARGB32);

    int sw = source.width();
    int sh = source.height();
    int xs = (sw << 8) / width;
    int ys = (sh << 8) / height;
    quint32 *dst = reinterpret_cast<quint32*>(dest.bits());
    int stride = dest.bytesPerLine() >> 2;

    for (int y = 0, yi = ys >> 2; y < height; ++y, yi += ys, dst += stride) {
       const quint32 *src1 = reinterpret_cast<const quint32*>(source.scanLine(yi >> 8));
       const quint32 *src2 = reinterpret_cast<const quint32*>(source.scanLine((yi + ys / 2) >> 8));
       for (int x = 0, xi1 = xs / 4, xi2 = xs * 3 / 4; x < width; ++x, xi1 += xs, xi2 += xs) {
           quint32 pixel1 = AVG(src1[xi1 >> 8], src1[xi2 >> 8]);
           quint32 pixel2 = AVG(src2[xi1 >> 8], src2[xi2 >> 8]);
           dst[x] = AVG(pixel1, pixel2);
       }
    }
    return dest;
}

QStringList getEntryList(QString path, QDir::Filter typeFilter, QStringList nameFilters){
    return QDir(path).entryList(nameFilters, typeFilter);
}

ActorList MapToList(ActorMap actors){
    ActorList list = {};
    QMapIterator<QString, ActorPtr> it(actors);
    while(it.hasNext()){
        it.next();
        list.push_back(it.value());
    }
    return list;
}

QString listToString(QStringList list){
    QStringListIterator it(list);
    QString s("");
    while (it.hasNext()){
        s.append(it.next());
        if (it.hasNext()){  s.append(", ");  }
    }
    return s;
}

QString system_call(QString command){
    QString output("");
    char buffer[4096];
    FILE *pipe = popen(qPrintable(command), "r");
    if (!pipe){
        qCritical("Runtime Error - popen() failed on command '%s'!", qPrintable(command));
    }
    try{
        while(!feof(pipe)){
            if (fgets(buffer, 4096, pipe) != NULL)
                output.append(buffer);
        }
    } catch (...) {
        pclose(pipe);
        qCritical("Caught Unknown Error While reading output of command '%s'!", qPrintable(command));
    }
    pclose(pipe);
    return output;
}

bool system_call_blocking(QString command, QStringList args, QString &output){
    bool success = false;
    QProcess *process = new QProcess();
    if (args.isEmpty()){
        args << "";
    }
    try{
        process->start(command, args);
        if (!process->waitForStarted()){
            qWarning("Error Starting QProcess with command '%s'", qPrintable(command));
        } else if (!process->waitForFinished()){
            qWarning("QProcess Timed out waiting for %s", qPrintable(command));
        } else {
            success = true;
        }
        output = process->readAllStandardOutput();
    } catch (std::exception &e){
        qWarning("Error Running Command %s: %s", qPrintable(command), e.what());
    }

    delete process;
    return success;
}

bool system_call_blocking(QString command, QStringList args){
    QString output("");
    return system_call_blocking(command, args, output);
}

bool wordDateToStruct(QString s, QDate &d){
    bool success = true;
    QRegularExpression dateRx("[A-Za-z.]+ (\\d{1,2})[rndsth]*[,]? (\\d{4})");
    QRegularExpressionMatch dateMatch = dateRx.match(s);
    if (dateMatch.hasMatch()){
        QString dayStr  = dateMatch.captured(1);
        QString monthStr= dateMatch.captured(2);
        QString yearStr = dateMatch.captured(3);
        // Convert to integers
        int dayInt = 0, monthInt = 0, yearInt = 0;
        dayInt = dayStr.toInt();
        yearInt = yearStr.toInt();
        if (monthStr.contains("Jan")){
            monthInt = 1;
        } else if (monthStr.contains("Feb")) {
            monthInt = 2;
        } else if (monthStr.contains("Mar")) {
            monthInt = 3;
        } else if (monthStr.contains("Apr")) {
            monthInt = 4;
        } else if (monthStr.contains("May")) {
            monthInt = 5;
        } else if (monthStr.contains("Jun")) {
            monthInt = 6;
        } else if (monthStr.contains("Jul")) {
            monthInt = 7;
        } else if (monthStr.contains("Aug")) {
            monthInt = 8;
        } else if (monthStr.contains("Sept")) {
            monthInt = 9;
        } else if (monthStr.contains("Oct")) {
            monthInt = 10;
        } else if (monthStr.contains("Nov")) {
            monthInt = 11;
        } else if (monthStr.contains("Dec")) {
            monthInt = 12;
        }
        if (monthInt > 0){
            success = true;
        }
        d = QDate(yearInt, monthInt, dayInt);
    }
    return success;
}
