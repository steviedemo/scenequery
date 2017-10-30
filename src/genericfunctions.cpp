#include "genericfunctions.h"
#include <QStringList>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>
#include <QDate>



QStringList getEntryList(QString path, QDir::Filter typeFilter, QStringList nameFilters){
    return QDir(path).entryList(nameFilters, typeFilter);
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
    bool result = false;
    QRegularExpression dateRx("[A-Za-z.]+ (\\d{1,2})[rndsth]*[,]? (\\d{4})");
    QRegularExpressionMatch dateMatch = dateRx.match(s);
    if (dateMatch.hasMatch()){
        result = true;
        QDate::fromString(s, Qt::DateFormat::)
    }
    boost::regex dateRx("[A-Za-z.]+ (\\d{1,2})[rndsth]*[,]? (\\d{4})");
    boost::smatch dateMat;
    bool success = false;
    if (boost::regex_search(s, dateMat, dateRx))
    //if (dateMat.hasMatch())
    {
        QString day_str, year_str;
        day_str = dateMat[1].str();
        year_str = dateMat[2].str();
      //  day_str = dateMat.captured(1).toStdString();
      //  year_str = dateMat.captured(2);
        if (!day_str.empty())
            d.setDay(atoi(day_str.c_str()));
        else
            return false;

        if (!year_str.empty())
            d.setYear(atoi(year_str.c_str()));
        else
            return false;

        if (d.y() < 50)
            d.setYear(d.y() + 1900);

        if (d.y() < 1950)
            return false;

        if (s.find("Jan") != QString::npos)
        {	d.setMonth(1); success = true;	}
        else if (s.find("Feb") != QString::npos)
        {	d.setMonth(2); success = true;	}
        else if (s.find("Mar") != QString::npos)
        {	d.setMonth(3); success = true;	}
        else if (s.find("Apr") != QString::npos)
        {	d.setMonth(4); success = true;	}
        else if (s.find("May") != QString::npos)
        {	d.setMonth(5); success = true;	}
        else if (s.find("Jun") != QString::npos)
        {	d.setMonth(6); success = true;	}
        else if (s.find("Jul") != QString::npos)
        {	d.setMonth(7); success = true;	}
        else if (s.find("Aug") != QString::npos)
        {	d.setMonth(8); success = true;	}
        else if (s.find("Sept") != QString::npos)
        {	d.setMonth(9); success = true;	}
        else if (s.find("Oct") != QString::npos)
        {	d.setMonth(10); success = true;	}
        else if (s.find("Nov") != QString::npos)
        {	d.setMonth(11); success = true;	}
        else if (s.find("Dec") != QString::npos)
        {	d.setMonth(12); success = true;	}
        //cout << d.year <<":"<<d.month<<":"<<d.day<<endl;
    }
    return success;
}
