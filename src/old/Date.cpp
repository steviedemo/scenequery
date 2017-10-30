#include <algorithm>
#include <boost/regex.hpp>
#include <iostream>
#include <QString>
#include <vector>
#include <QRegularExpression>

#include "Date.h"
#include "Scene.h"
#include "Filepath.h"   
#include "structs.h"
#include "stringTools.h"
#define BUFFSIZE 4096

static const QString monthNamesAbbr[13] = {"INVALID", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
static const QString monthNames[13] = 	 {"INVALID", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
static const QString legal_digits = "0123456789";

// Date Constructors
Date::Date (void){
	year = 0;
	month = 0;
	day = 0;
} 
Date::Date (vector<int> d)	{	set(d);			}							// Construct from Int Vector
Date::Date (int y, int m, int d)	{	set(y, m, d);	}							// Construct from 3 ints
Date::Date (int y, int m)			{	set(y, m, 15);	}							// Construct from 2 ints
Date::Date (int y)				{	set(y);			}							// Construct from 1 int
Date::Date (vector<QString> d)					{	set(d);			}	// Construct from String Vector
Date::Date (QString y, QString m, QString d)	{	set(y, m, d);	}	// Construct from 3 QStrings	
Date::Date (QString y, QString m)					{	set(y, m);		}	// Construct from 2 QStrings
Date::Date (QString d)								{	set(d);			}	// Construct from 1 QString
Date::Date (const Date &date){			// Copy Constructor
	year = date.year;
	month = date.month;
	day = date.day;
}


void Date::illegalCharMsg(QString s){
	cerr<< "\nError: Couldn't convert " << s << " To Date type because it contains an illegal character.\n\n";
}


Date todaysDate(void){
	QDateTime d = QDateTime::currentDateTime();
	Date today(d.toString("yyyy"), d.toString("mm"), d.toString("dd"));
	return today;
}

bool isDateValid(Date d){
	bool v = true;
	if (d.y() < 1950){
		v = false;
	}else if (d.m() < 1 || d.m() > 12){
		v = false;
	}else if (d.d() < 1 || d.d() > 31){
		v = false;
	}
	return v;
}

int age(Date &d, int year){
	if (d.y() > year)
		return d.y() - year;
	else
		return year - d.y();
}

int age(Date &d, Scene &s){
	if (s.hasReleaseDate())
	{
		Date d2 = s.getReleaseDate();
		return age(d, d2);
	}
	else if (s.getYear() > 0)
	{
		return age(d, s.getYear());
	}
	else
	{
		return -1;
	}
}

int age(Date date1, Date date2){
	int years_apart = -1;
	Date d1, d2;
	if (date1.hasYear() && date2.hasYear())	
	{	// ensure d2 is the later date, so we don't get a negative age.
		if (date1.before(date2)){
			d1 = date1;
			d2 = date2;
		}else{
			d1 = date2;
			d2 = date1;
		}
		if (d1.hasDay() && d2.hasDay())
		{
			if (d2.m() == d1.m())	// Comes down to days
			{
				if (d2.d() >= d2.d())
					years_apart = d2.y() - d1.y();
				else
					years_apart = d2.y() - d1.y() + 1;
			}
			else if (d2.m() > d1.m()){
				years_apart = d2.y() - d1.y();
			}
			else{
				years_apart = d2.y() - d1.y() + 1;
			}
		}
		else if (d1.hasMonth() && d2.hasMonth())
		{
			if (d2.m() >= d1.m()){
				years_apart = d2.y() - d1.y();
			}
			else{
				years_apart = d2.y() - d1.y() + 1;
			}	
		}
		else
		{
			years_apart = d2.y() - d1.y();
		}
	}
	/*
	else 
	{
		cout << "invalid dates" << endl;
	}
	*/
	return years_apart;
}

QString monthNameAbbr(int i){
	if (i > 0)
		return QString(monthNamesAbbr[i]);
	else
		return "";
}

QString monthName(int i ){
	QString retStr("");
	if (i > 0)
		retStr = QString(monthNames[i]);
	return retStr;
}

Date textDateToStruct(QString s){
    Date d;
    QRegularExpression re("(\\d+)[/:.-](\\d+)[/.:-](\\d+)");
    QRegularExpressionMatch match = re.match(s.c_str());
    if (match.hasMatch())
    {
        d.setYear(match.captured(1).toInt());
        d.setMonth(match.captured(2).toInt());
        d.setDay(match.captured(3).toInt());
    }
    return d;
}

bool wordDateToStruct(QString s, Date &d){
//    QRegularExpression dateRx("[A-Za-z.]+ (\\d{1,2})[rndsth]*[,]? (\\d{4})");
//    QRegularExpressionMatch dateMat = dateRx.match(s.c_str());
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

Date Date::todaysDate(){

}

bool Date::isDate(std::string s){
    bool yes = true;
    for (size_t i = 0; i < s.size() && !yes; ++i){
        char c = s.at(i);
        if (!isdigit(c) && (c != '-') && (c != '.') && (c != '/'))
            yes = false;
    }
    return yes;
}
Date Date::parseDate(QString s){
    Date d;
    if (!s.isEmpty() && contains(s, "20")){
        if (s.size() == 4){
            d.set(s);
        } else {
            vector<string> temp = tokenizeString(s, '.');
            d.set(temp);
        }
    }
    return d;
}
Date Date::parseDate(Filepath f){
    Date d;
    string date_str = parseParenthData(f, DATE_OPT);
    if (!date_str.isEmpty())
    {
        d = Date(date_str);
    }
    return d;
}

Date wordDateToStruct(QString s){
	Date d;
	wordDateToStruct(s, d);
	return d;
}

QString sqlSafe(Date d)
{
    QString s("'"+d.ymd('-')+"'");
    return s;
}

void	Date::lock 	(void) 	{ 	locked = true;	}										// Lock the Struct to Prevent further changes.
void	Date::unlock 	(void)	{	locked = false;	}										// Unlock the Struct.
// Return a date Structure
Date Date::date (void)
{
    Date d(year, month, day);
    return d;
}

bool Date::set (vector<int> d, bool lockAfter){
	bool success = false;
	try{
		if(!locked){
		    if (d.size() > 2)		setDay(d.at(2));
		    if (d.size() > 1)		setMonth(d.at(1));
		    if (d.size() > 0)		success = setYear(d.at(0));
		}else{	cerr << "Attempting to modify locked date " << toString() << " with int vector containing" << d.at(0) << endl;}
	}catch(exception &e){cerr << "Error Converting QString vector to date in " << __FUNCTION__ << ": " << e.what();}
	if (success && lockAfter)
	{
		locked = true;
	}
	return success;
}
bool Date::set(int y, int m, int d, bool lockAfter){
	bool success = false;
	if(!locked){
		success = setYear(y);
		setMonth(m);
		setDay(d);
	}else{	cerr << "Attempting to modify locked date " << toString() << " with 3 ints: " << y << ", " << m << ", " << d << endl;}
	if (success && lockAfter)
	{
		locked = true;
	}
	return success;
}
bool Date::set(int y, int m, bool lockAfter){
	bool success = false;
	if(!locked){
		success = setYear(y);
		setMonth(m);
	}else{	cerr << "Attempting to modify locked date " << toString() << " with 2 ints: " << y << ", " << m << endl;}
	if (success && lockAfter)
	{
		locked = true;
	}
	return success;
}
bool Date::set(int y, bool lockAfter){
	bool success = false;
	if(!locked){
		success = setYear(y);
	}else{	cerr << "Attempting to modify locked date " << toString() << " with 1 int: " << y << endl;}
	if (success && lockAfter)
	{
		locked = true;
	}
	return success;
}
bool Date::set (vector<QString> d, bool lockAfter){
	bool success = false;
	try{
		if (!locked && d.size() > 0){
		    if (d.size() > 2)
		    		success = set(d.at(0), d.at(1), d.at(2));
		    else if (d.size() > 1)
		    		success = set(d.at(0), d.at(1));
		    else
		    		success = setYear(d.at(0));
		}else{	cerr << "Attempting to modify locked date " << toString() << " with QString vector: " << d.at(0) << "-" << d.at(1) << "-" << d.at(2) << endl;}
	}catch(exception &e){cerr << "Error Converting QString vector to date in " << __FUNCTION__ << ": " << e.what();}
	if (success && lockAfter)
	{
		locked = true;
	}
	return success;
}
bool Date::set (QString y, QString m, QString d, bool lockAfter){
	bool success = false;
	if(!locked && !y.empty()){
		try{
			success = setYear(y);	// Only The Year need be successfully set for this function to return true.
		    //success &= setMonth(m);
		    setMonth(m);
		    //success &= setDay(d);
		    setDay(d);
		}catch(exception &e){cerr << "Error Converting QString vector to date in " << __FUNCTION__ << ": " << e.what();}
	}else{	cerr << "Attempting to modify locked date: " << toString() << " with 3 QStrings: " << y << ", " << m << ", " << d << endl;}
	// Provide Option to Lock Automatically upon successful setting.
	if (success && lockAfter)
	{
		locked = true;
	}
	return success;
}
bool Date::set (QString y, QString m, bool lockAfter){
	bool success = false;
	if (!locked){
		try{
			success = setYear(y);
			setMonth(m);
			setDay(15);	// Automatically set to middle of the month.
		}catch(exception &e){cerr << "Error Converting QString to date in " << __FUNCTION__ << ": " << e.what();}
	}else{	cerr << "Attempting to modify locked date: " << toString() << " with 2 QStrings " << y << ", " << m <<  endl;}
	if (success & lockAfter)
	{
		locked = true;
	}
	return success;
}
// NOT used to just set a year (although it would do that too). Meant to interperet a full date QString.
bool Date::set (QString date_str, bool lockAfter){
	bool success = false;
	// Date can't be locked, String can't be empty, and can only contain digits, periods and hyphens. 
	if(!locked && !date_str.empty())
	{
		if (!legalDateString(date_str))
			illegalCharMsg(date_str);
		else{
			try{
				vector<QString> parts = recursiveTokenizer(date_str, ".-");	// split QString by periods or dashes.
				// now have parts of date in separate elements, but no idea how many elements. We know they are valid, though.
				success = set(parts);	// Send vector to the setter that takes QString vectors.
			}catch(exception &e){cerr << "Error Converting QString to date in " << __FUNCTION__ << ": " << e.what();}
		}
	}else{	cerr << "Attempting to modify locked date: " << toString() << " with date QString " << date_str << endl;}
	if (success && lockAfter)
	{
		locked = true;
	}
	return success;
}
// Receive:	String
// Check:	String not empty. String only contains 
bool Date::setYear(QString y_str){
	bool success = true;
	if (!y_str.empty() && legalNumberString(y_str)){
		try{
			int y_int = atoi(y_str.c_str());
			success = setYear(y_int);
		}catch(exception &e){cerr << "Error Converting QString \"" << y_str << "\" to year in " << __FUNCTION__ << ": " << e.what();}
	}else{success = false;}
	return success;
}
bool Date::setMonth(QString m_str){
	bool success = true;
	if (!m_str.empty() && legalNumberString(m_str)){
		try{
			int m_int = atoi(m_str.c_str());
			success = setMonth(m_int);
		}catch(exception &e){cerr << "Error Converting QString \"" << m_str << "\" to month in " << __FUNCTION__ << ": " << e.what();}
	}else{success = false;}
	return success;
}
bool Date::setDay(QString d_str){
	bool success = true;
	if (!d_str.empty() && legalNumberString(d_str)){
		try{
			int d_int = atoi(d_str.c_str());
			success = setMonth(d_int);
		}catch(exception &e){cerr << "Error Converting QString \"" << d_str << "\" to day in " << __FUNCTION__ << ": " << e.what();}
	}else{success = false;}
	return success;
}
// Receive: Int
// Check:	That Values are in the proper ranges.
bool Date::setYear(int y){
	bool success = false;
	if (y > 100)	{	
		year = y;
		success = true;
	}
	else if (y > 0 && y < 100){
		year = y + 1900;
		success = true;
	}
	else{
		cerr << "Invalid year entered: " << y << endl;
	}
	return success;
}
// Set Month
bool Date::setMonth(int m){
	bool success = false;
	if (m > 0 && m < 13){	
		month = m;
		success = true;
	}
	return success;
}
// Set Day
bool Date::setDay (int d){
	bool success = false;
	if (month == 2 && d > 29)
	{
		return false;
	}
	else if (d > 0 && d < 32)
	{
		day = d;
		success = true;
	}
	return success;
}

//--------------------------------------------
// QUERY FUNCTIONS
//--------------------------------------------
bool Date::hasYear(void){
	bool has = false;
	if (year > 0)
		has = true;
	return has;
}
bool Date::hasMonth(void){
	bool has = false;
	if (month > 0 && month < 13)
	    has = true;
	return has;
}
bool Date::hasDay(void){
	bool has = false;
	if (day > 0)
	    has = true;
	return has;
}
int Date::size(void)
{
	int count = 0;
	if (hasDay())	++count;
	if (hasMonth())	++count;
	if (hasYear())	++count;
	return count;
}
bool Date::empty(void)	{	
	bool empty = false;
	if (size() == 0)
		empty = true;
	return empty;
}

bool Date::sameDay(Date d){
	if (d.d() == day)	{	return true;	}
	else				{	return false;	}
}
bool Date::sameMonth(Date d){
	if (d.m() == month)	{	return true;	}
	else				{	return false;	}
}
bool Date::sameYear(Date d){
	if (d.y() == year)	{	return true;	}
	else				{	return false;	}
}
bool Date::on(Date d){
	if (sameYear(d) && sameMonth(d) && sameDay(d))
		return true;
	else
		return false;
}
bool Date::before(Date d)	// Returns true if 'this' date is chronologically before the date provided.
{
	if (year < d.y())
		return true;
	else if (year > d.y())
		return false;
	else	// Equal Year
	{
		if (month < d.m())
			return true;
		else if (month > d.m())
			return false;
		else	// Equal Month
		{
			if (day < d.d())
				return true;
			else
				return false;
		}
	}
}
bool Date::beforeOrOn(Date d)	// Returns true if 'this' date is chronologically before the date provided.
{
	if (year < d.y())
		return true;
	else if (year > d.y())
		return false;
	else	// Equal Year
	{
		if (month < d.m())
			return true;
		else if (month > d.m())
			return false;
		else	// Equal Month
		{
			if (day <= d.d())
				return true;
			else
				return false;
		}
	}
}
bool Date::after(Date d)
{
	if (year > d.y())
		return true;
	else if (year < d.y())
		return false;
	else	// Equal Year
	{
		if (month > d.m())
			return true;
		else if (month < d.m())
			return false;
		else	// Equal Month
		{
			if (day > d.d())
				return true;
			else
				return false;
		}
	}	
}	
bool Date::afterOrOn(Date d)
{
	if (year > d.y())
		return true;
	else if (year < d.y())
		return false;
	else	// Equal Year
	{
		if (month > d.m())
			return true;
		else if (month < d.m())
			return false;
		else	// Equal Month
		{
			if (day >= d.d())
				return true;
			else
				return false;
		}
	}	
}	

//--------------------------------------------
// DATA RETREIVAL FUNCTIONS
//--------------------------------------------
int Date::y(void)		{	return year;		}
int Date::m(void)		{	return month;		}
int Date::d(void)		{	return day;			}

QString Date::ymd(char c){
    QString date("");
    if (year > 1500)
    {
		date += to_QString(year);
		if (hasMonth()){
			date += c;
			if (month < 10)
			    date += '0';	
			date += to_QString(month);
		};
		if (hasDay()){
			date += c;
			if (day < 10)
				date += '0';
			date += to_QString(day);
		}
    }
    return date;
}
QString Date::toString(char c){
	return ymd(c);
}
QString Date::mdy(char c){
	return to_QString(month)+c+to_QString(year)+c+to_QString(day);
}
QString Date::sql(void){return "'"+to_QString(year)+"-"+to_QString(month)+"-"+to_QString(day)+"'";}
QString Date::wordString(void)
{	
	QString phrase("");
	if (hasMonth() && month < 13 && month > 0)
		phrase += monthNames[month] + " ";
	if (hasDay())
		phrase += to_QString(day) + ", ";
	if (hasYear())
		phrase += to_QString(year);
	return phrase;
}

void initDate(void){todays_date = todaysDate();}
