#include "genericfunctions.h"
#include <QStringList>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>
#include <QDate>



QStringList getEntryList(QString path, QDir::Filter typeFilter, QStringList nameFilters){
    return QDir(path).entryList(nameFilters, typeFilter);
}

QString headshotName(QString actorName){
    QString title("");
    actorName = actorName.trimmed();
    for (int i = 0; i < actorName.size(); ++i){
        QChar c = actorName.at(i);
        if (c == ' '){
            title.push_back('_');
        } else {
            if (c.isLetter()){
                title.push_back(c);
            }
        }
    }
    return title;
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
//        throw std::runtime_error("popen() failed!");
    }
    try{
        while(!feof(pipe)){
            if (fgets(buffer, 4096, pipe) != NULL)
                output.append(buffer);
        }
    } catch (...) {
        pclose(pipe);
        qCritical("Caught Unknown Error While reading output of command '%s'!", qPrintable(command));
        //throw;
    }
    pclose(pipe);
    return output;
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
