#ifndef __DATE_H__
#define __DATE_H__

#include <vector>
#include <QString>
using namespace std;
// Non-Member Date Functions

QString     sqlSafe(class Date);
bool		wordDateToStruct(QString s, class Date &d);
class Date 	wordDateToStruct(QString s);
class Date	textDateToStruct(QString s);
void		initDate();
void		initDate(void);
class Date{
public:
	Date (void);
	Date (const Date &);
    Date (vector<QString>);
    Date (vector<int>);
    Date (QString);
    Date (QString, QString);
    Date (QString, QString, QString);
	Date (int);
	Date (int, int);
	Date (int, int, int);

//------------------------------
// 	SETTERS
//------------------------------
	void lock(void);
	void unlock(void);
	bool isLocked(void);

    bool set (vector<int>, bool lockAfter = false);
    bool set (vector<QString>, bool lockAfter = false);
    bool set (QString,  bool lockAfter = false);
    bool set (QString, QString,  bool lockAfter = false);
    bool set (QString, QString, QString,  bool lockAfter = false);
	bool set (int,  bool lockAfter = false);
	bool set (int, int,  bool lockAfter = false);
	bool set (int, int, int, bool lockAfter = false);

    bool setYear(QString);
    bool setMonth(QString);
    bool setDay(QString);
	bool setYear(int);
	bool setMonth(int);
	bool setDay(int);

//------------------------------
// 	GETTERS
//------------------------------	
	int y(void);
	int m(void);
	int d(void);

    QString ymd(char c = '-');
    QString mdy(char c = '-');
    QString toString(char c = '-');
    QString sql(void);
    QString wordString(void);
	Date date (void);
//------------------------------
//	CHECKERS
//------------------------------	

	bool hasYear(void);
	bool hasMonth(void);
	bool hasDay(void);

	int  size(void);
	bool isEmpty(void);
    bool sameDay(Date);
	bool sameMonth(Date);
	bool sameYear(Date);
	bool on(Date);
	bool after(Date);
	bool before(Date);
	bool beforeOrOn(Date);
	bool afterOrOn(Date);
// Static Functions
    static bool isDate(std::string);
    static bool isDate(QString s){ return isDate(s.toStdString()); }
    static Date parseDate(QString);
    static Date parseDate(class Filepath);
    static Date todaysDate(void);

private:
    void illegalCharMsg(QString);
	int year = 0;
	int month = 0;
	int day = 0;
	bool locked = false;
};

#endif
