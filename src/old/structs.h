#ifndef __STRUCTS_H__
#define __STRUCTS_H__
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "Date.h"
#define A_VAL 7
#define B_VAL 4
#define C_VAL 1
#define DEFAULT_INT 	-1
#define DEFAULT_DOUBLE 	0.0
#define DEFAULT_STRING 	""

//--------------------------------------------------------------------
//  STRING_COUNT_T
//--------------------------------------------------------------------
typedef struct string_count_t {
	private:
		int count = 1;
	public:
		std::string name;
		string_count_t(std::string n, int c)
		{
			name = n;
			count = c;
		}
		string_count_t(std::string n)
		{
			name = n;
			count = 1;
		}
		string_count_t(void)
		{
			name = DEFAULT_STRING;
			count = 1;
		}
		int 	getCount() 	{	return count;	}
		void 	increment()	{ 	count++;		}
		void 	decrement()	{ 	count--;		}
		bool	empty()		{	return name.empty();	}
} string_count_t;

//--------------------------------------------------------------------
//  RATING_T
//--------------------------------------------------------------------


typedef struct rating_t{
private:
    std::string rating;
    unsigned int n;
    unsigned int strToNum(std::string r)
    {// Assume this is a valid rating value
        int num = 0;
        for (std::string::iterator s=r.begin(); s != r.end(); ++s)
        {
            if (*s == 'C' || *s == 'R')
                num += C_VAL;
            else if (*s == 'B')
                num += B_VAL;
            else if (*s == 'A')
                num += A_VAL;
            else if (*s == '-')
                --num;
            else if (*s == '+')
                ++num;
        }
        return num;
    }
public:
    rating_t(void)
    {
        rating=DEFAULT_STRING;
        n = 0;
    }
    rating_t(std::string r)
    {
        rating = r;
        n = strToNum(r);
    }
    std::string toString(void)
    {
        return rating;
    }
    unsigned int toInt(void)
    {
        return n;
    }
    void set(std::string s) 
    {
        n = strToNum(s);
        rating = s;
    }
    bool greaterThan(rating_t other)
    {
        if (n > other.toInt())
            return true;
        else
            return false;
    }
    bool lesserThan(rating_t other)
    {
        if (n < other.toInt())
            return true;
        else
            return false;
    }
    bool empty(void)
    {
        if (rating.empty())
            return true;
        else
            return false;
    }
}rating_t;

//--------------------------------------------------------------------
//  ABBR_T
//--------------------------------------------------------------------
typedef struct abbr_t{
private:
	std::string abbrv, full;
public:
	abbr_t(std::string f, std::string a){
		full = f;
		abbrv = a;
	}
	std::string word(void){	return full; }
	std::string abbr(void){	return abbrv; }
	bool empty(void){
	    if (abbrv.empty() || full.empty())
		return true;
	    else
		return false;
	}
} abbr_t;

//--------------------------------------------------------------------
//  HEIGHT_T
//--------------------------------------------------------------------


typedef struct height_t {
	int feet = DEFAULT_INT;
	int inches = DEFAULT_INT;
	height_t() : feet(DEFAULT_INT), inches(DEFAULT_INT)	{}
	
	height_t(int f, int i)	{	set(f, i);	}
	height_t(int cm)		{	set(cm);	}
	void set(int cm)
	{
		int total_inches = cm/2.54;
		feet = total_inches / 12;
		inches = total_inches % 12;	
	}
	void set(int f, int i)
	{
		feet = f;
		inches = i;
	}
	int toInches(void){
		return feet*12 + inches;
	}
	int toCm(void){
		return 2.54 * ((feet*12) + inches);
	}
	double toDouble(void){
		return (double)((double)feet + ((double)inches/12));
	}
	std::string toString(void){
		return std::to_string(feet) + " feet " + std::to_string(inches) + " inches";
	}
	bool nonzero(void)
	{
		if (feet > 0 || inches > 0)
			return true;
		else
			return false;
	}
	bool empty(void){
	    if (feet == DEFAULT_INT)
		return true;
	    else
		return false;
	}
} height_t;

//--------------------------------------------------------------------
//  CONTENT_T 
//--------------------------------------------------------------------

typedef struct content_t{
	std::string tag, content;
} content_t;

//--------------------------------------------------------------------
//  BIO_T
//--------------------------------------------------------------------


typedef struct bio_t {
	bool active = false;
	int start = 				DEFAULT_INT;
	int end = 					DEFAULT_INT;
	int weight = 				DEFAULT_INT;
	Date birthdate;
	height_t height;
	std::string name = 			DEFAULT_STRING; 
	std::string aliases= 		DEFAULT_STRING;
	std::string city= 			DEFAULT_STRING;
	std::string ethnicity= 		DEFAULT_STRING;
	std::string nationality= 	DEFAULT_STRING;
	std::string eyeColor= 		DEFAULT_STRING;
	std::string hairColor= 		DEFAULT_STRING;
	std::string measurements= 	DEFAULT_STRING;
	std::string tattoos= 		DEFAULT_STRING;
	std::string piercings= 		DEFAULT_STRING;
	
	void addBirthday(std::string b)	{ 	birthdate 	= wordDateToStruct(b); }
	void addCareer(int b, int e)	{	start = b; end = e;			}
	void addHeight(int f, int i)	{	height = height_t(f, i);	}
	bool empty(void)
	{
		if ((hairColor == DEFAULT_STRING) && (measurements == DEFAULT_STRING))
			return true;
		else
			return false;
	}
	std::string toString(void){
		std::string o(DEFAULT_STRING);
		if (!aliases.empty())						{	o += "\tAliases:\t" 	+ 	aliases + "\n";						}
		if (birthdate.hasMonth())					{	o += "\tBorn:\t\t" 		+ 	birthdate.wordString() + "\n";		}
		if (!nationality.empty() && !city.empty())	{	o += "\tBirthplace:\t" 	+ 	city + ", " + nationality + "\n";	}
		else if (!city.empty())						{	o += "\tBirthplace:\t" 	+ 	city + "\n";						}
		else if (!nationality.empty())				{	o += "\tBirthplace:\t" 	+ 	nationality + "\n";					}
		if (!ethnicity.empty())						{	o += "\tEthnicity:\t" 	+ 	ethnicity + "\n";					}
		if (!hairColor.empty())						{	o += "\tHair:\t\t" 		+ 	hairColor + "\n";					}
		if (!eyeColor.empty())						{	o += "\tEyes:\t\t" 		+ 	eyeColor + "\n";					}
		if (height.nonzero())						{	o += "\tHeight:\t\t" 	+ 	height.toString() + "\n";			}
		if (weight > 0)								{	o += "\tWeight:\t\t" 	+ 	std::to_string(weight)+" lbs."+"\n";}
		if (!measurements.empty())					{	o += "\tMeasurements:\t"+ 	measurements + "\n";				}
		if (!tattoos.empty())						{	o += "\tTattoos:\t" 	+ 	tattoos + "\n";						}
		if (!piercings.empty())						{	o += "\tPiercings:\t" 	+ 	piercings + "\n";					}
		return o;
	}
	int count(void){
		int count = 0;
		if (!aliases.empty())		{	++count;	}	// 1
		if (birthdate.hasMonth())	{	++count;	}	// 2
		if (!nationality.empty())	{	++count;	}	// 3
		if (!hairColor.empty())		{	++count;	}	// 4
		if (!eyeColor.empty())		{	++count;	}	// 5
		if (height.nonzero())		{	++count;	}	// 6
		if (weight > 0)				{	++count;	}	// 7
		if (!measurements.empty())	{	++count;	}	// 8
		if (!tattoos.empty())		{	++count;	}	// 9
		if (!piercings.empty())		{	++count;	}	// 10
		if (!ethnicity.empty())		{	++count;	}	// 11
		if (!city.empty())			{	++count;	}	// 12
		return count;
	}
	void print(void){
		std::cout << toString() << std::endl;
	}
} bio_t;


//--------------------------------------------------------------------
//  DVD_T
//--------------------------------------------------------------------

typedef struct DVD_INFO {
	std::string title, cast, studio;
	int seriesNum, sceneCount;
	Date releaseDate;
	double rating, length;
	//class Filepath poster;
} dvd_t;

typedef struct actorQuery{
	std::string field;
	std::string term;
	int number;
} actorQuery_t;

typedef struct sceneQuery{
	std::string field;
	std::string term;
	int number;
} sceneQuery_t;

//typedef struct searchOpts {
//	bool set = false;
//	std::string query = DEFAULT_STRING;
//	bool lt = false;
//	bool gt = false;
//	bool size = false;
//	bool length = false;
//	double number;
//	bool ratedOnly = false;
//} search_t;

//typedef struct runOptions {
//	bool recursive = true;
//	bool interactive = false;
//	bool silent = false;
//	bool test = false;
//	bool newSceneDb = false;
//	bool updateSceneDb = false;
//	bool newActorDb = false;
//	bool updateActorDb = false;
//	bool useDatabases = false;
//	bool queryActors = false;
//	bool findActor = false;
//	bool queryScenes = false;
//	bool printActresses = false;
//	bool printScenes = true;
//	bool outputToFile = false;
//	bool getAges = false;
//	std::string outputLog = DEFAULT_STRING;
//	sceneQuery_t sceneQuery;
//	actorQuery_t actorQuery;
//} runOptions_t;

//--------------------------------------------------------------------
//  GENERAL FUNCTION PROTOTYPES
//--------------------------------------------------------------------


bool							strCountSort			(string_count_t, string_count_t);
height_t				 		stringToHeight			(std::string s);
std::string 					heightToString			(height_t h);
height_t 						wordsToHeight			(std::string s);
int 							totalInches 			(height_t h);
bool							updateStringCounts		(std::string, std::vector<string_count_t> &);
void							printStringCountList	(std::vector<string_count_t> &);
bool							validHeight				(height_t);
std::string						toWords					(height_t);
std::vector<string_count_t> 	getCompanyList			(std::vector<class Scene> &);
int 		age				(Date &, class Scene &);
int 		age				(Date &, int);
int			age				(Date , Date );
std::string toSqlString		(int, int, int);

#endif
