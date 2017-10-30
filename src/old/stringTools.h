#ifndef __STRING_TOOLS_H__
#define __STRING_TOOLS_H__

#include <string>
#include <vector>
#include <QString>
bool        emp           (std::string s);
// String Editing Tools

void        trimWhitespace  (std::string &);
bool        contains        (std::string, std::string);
std::string stripString     (std::string);
std::string getTabString    (std::string, int);
std::string addTabs			(std::string, int);

bool 		legalDate 		(std::string test, std::string legalChars);
bool		legalDateString	(std::string);
bool 		legalNumberString(std::string);
std::string unixSafe        (std::string s);
QString     sqlSafe         (std::string s);
QString     sqlSafe         (int);
QString     sqlSafe         (double);
std::string toTitle         (std::string);
std::string properName      (std::string);
std::string getExtension    (std::string);
std::string getBasename     (std::string);
std::string toImage         (std::string, std::string ext = "jpeg");
std::string toImageFormatString(std::string name, std::string ext="png");

 
void        removeChar      (std::string &, char);
bool        removeStr       (std::string &, std::string, bool preChar = false);
bool        removeStr       (std::string &, char, bool preChar = false);
std::string removeBefore    (std::string, char, int times = 1);
std::string removeBefore    (std::string, std::string, int times = 1);
std::string removeAfter     (std::string, char, int times = 1);
std::string removeAfter     (std::string, std::string, int times = 1);

std::string replaceString 	(std::string full, std::string toSwapOut, std::string toSwapIn);

std::string getBetween      (std::string, char, char);
std::string getBetween      (std::string, std::string, std::string);
std::vector<std::string>  tokenizeString      (std::string, std::string);
std::vector<std::string>  tokenizeString      (std::string, char);
std::vector<std::string>  tokenizeString      (std::string, std::vector<char>, int idx = 0);
std::vector<std::string>  recursiveTokenizer  (std::string, std::string, int idx = 0);
std::vector<std::string>  recursiveTokenizer  (std::string, std::vector<std::string>, int idx = 0);
#endif
