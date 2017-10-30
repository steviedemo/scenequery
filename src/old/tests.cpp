#include <algorithm>
#include <boost/regex.hpp>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <string>
#include <vector>
#include "Actor-old.h"
#include "Scene.h"
#include "curlRequests.h"
#include "database.h"
#include "lists.h"
#include "output.h"
#include "stringTools.h"
#include "tools.h"
#include "Date.h"
#include "Filepath.h"   
#include "structs.h"
#include "tests.h"
#include "imageTools.h"



using namespace std;

void testImageDownload(std::string name){
    downloadHeadshot(name);
    std::cout << "Done" << std::endl;
}

void testHeightAndWeight(std::string name)
{
    height_t h(0);
    int weight = 0;
    fetchHeightAndWeight(name, h, weight);
    std::cout << "Name:\t" << name << "\nHeight:\t" << h.toString() << "\nWeight:\t" << weight << std::endl;

}

void testRename(std::vector<Scene> &list)
{
    std::cout << "Testing Rename Function: \n\n";
    for (size_t i = 0; i < list.size(); i++)
    {
        std::cout << list.at(i).filename() << "\n\t--------------->\n" << newFilename(list.at(i)) << std::endl << std::endl;
    }
}
void testIAFDquery(std::string name)
{
    std::string html = getIAFDhtml(getIAFDURL(name));
    std::vector<std::string> htmlSeperators = {"</p>", "</div>", "</a>"};
    //std::vector<std::string> htmlvec = recursiveTokenizer(html, {"</p>", "</div>", "</a>"});
    std::vector<std::string> htmlvec = recursiveTokenizer(html, htmlSeperators);
    std::string bioline = getIAFDBioLine(htmlvec);
    std::cout << "Bio Line: " << bioline << std::endl;
    
    /*
    std::vector<Scene> list = getOnlineFilmography(html);
    printSceneList(list);
    std::vector<string_count_t> tally = getCompanyList(list);
    printStringCountList(tally);
    */
}   
void testAgeCalculator(Actor a, Scene s)
{
    Date bday = a.getBirthday();   
    Date rdate = s.getReleaseDate();
    if  (!a.hasBirthday())
        std::cout << a.getName() << " doesn't have a recorded birthday" << std::endl;
    else if (!s.hasReleaseDate())
        std::cout << s.getTitle() << " doesn't have a recorded release date." << std::endl;
    else{
        int ageIn = age(bday, rdate);
        if (ageIn > -1)
            std::cout << a.getName() << " was " << ageIn << " when she made " << s.getTitle() << std::endl;
        else           
            std::cout << "No Idea why this didn't work with " << a.getName() << " & " << s.getTitle() << std::endl;
    }
}
void testSqlString(std::string s)
{
    std::cout << "Testing SQL String Sanitizer:" << std::endl;
    std::cout << s << "--->" << sqlSafe(s) << std::endl;
}
void testIAFDquery(void)
{
    std::cout << getIAFDhtml("http://www.iafd.com/person.rme/perfid=jadejantzen/gender=f/jade-jantzen.htm") << std::endl;
}
void testFreeonesQuery(void)
{
    std::cout << fetchHTML("http://www.freeones.com/html/j_links/Jade_Jantzen/") << std::endl;
}
void testFreeonesQuery(std::string name)
{
    std::cout << fetchHTML(getFreeonesURL(name)) << std::endl;
}
void testBasename(std::string s)
{
    std::cout << s << " ---> " << getBasename(s) << std::endl;
}
void testRemoveString(std::string s, char r, bool start)
{
    std::cout << std::endl << "Removing " << r << " from ";
    if (start)
    {
        std::cout << " Start of " << s << std::endl;
    }
    else
    {   
        std::cout << " end of " << s << std::endl;
    }
    std::string newStr = s;
    removeStr(newStr, r, start);
    std::cout << s << "\n--->\n" << newStr << std::endl << std::endl; 
}
void testRemoveString2(std::string s, std::string r, bool start)
{
 
    std::cout << std::endl << "Removing " << r << " from ";
    if (start)
    {
        std::cout << " Start of " << s << std::endl;
    }
    else
    {   
        std::cout << " end of " << s << std::endl;
    }
    std::string newStr = s;
    removeStr(newStr, r, start);
    std::cout << s << "\n--->\n" << newStr << std::endl << std::endl; 
}
void testHeight(std::string s)
{
    height_t h = stringToHeight(s);
    std::cout << "Feet: " << h.feet << "\n" << "Inches: " << h.inches << std::endl;
    std::string h_string = heightToString(h);
    std::cout << h_string << std::endl;
}
void testDates(std::string s)
{
    Date d;
    if (wordDateToStruct(s, d))
    std::cout << s << " ---> " << d.mdy('/') << std::endl;
    else
    std::cout << "Error Converting " << s << " to date Structure" << std::endl;
}
void testTodaysDate()
{
    Date t = todaysDate();
    std::cout << "Today's Date is: " << t.ymd('/') << ", or " << t.wordString() << std::endl; 
}
void testAge(std::string s1, std::string s2)
{
    Date d1, d2;
    if (wordDateToStruct(s1, d1) && wordDateToStruct(s2, d2))
    {
        int diff = age(d1, d2);
        std::cout << "There are " << diff << " years between " << s1 << " & " << s2 << std::endl;
    }
    else
        std::cout << "Error converting " << s1 << " and/or " << s2 << " to date structures" << std::endl;
}
//-------------------------------------------  TESTS ------------------------------------------------------
void testTokenizer(std::string testStr, std::string delim)
{
    std::cout << "Testing tokenizeString with input: " << testStr << " and delimiter: " << delim << std::endl;
    std::vector<std::string> test = tokenizeString(testStr, delim); 
    for (int i = 0; i < (int)test.size(); i++)
    {
    std::cout << "Token #" << i+1 << ":\t" << test.at(i) << std::endl;
    }
    std::cout << "Test Complete.\n\n";
}
void testRTokenizer(std::string testStr, std::string delim)
{
    std::cout << "Testing recursiveTokenizer with input: " << testStr << " and delimiter: " << delim << std::endl;
    std::vector<std::string> test = recursiveTokenizer(testStr, delim, 0);
    for (int i = 0; i < (int) test.size(); i++)
    {
    std::cout << "Token #" << i+1 << ":\t" << test.at(i) << std::endl;
    }
    std::cout << "Test Complete\n\n";
}
 
    /*
void testParseCast(std::string filename)
{
    std::vector<std::string> names = parseCast(filename);
    std::cout << "Testing parseCast with the filename: " << filename << std::endl;
    for (int i = 0; i < names.size(); i++)
    {
    std::cout << "Name #" << i << ":\t" << names.at(i) << std::endl;
    }
    std::cout << "Testing of parseCast complete" << std::endl << std::endl;
}
void testParseTitle(std::string filename)
{
    
    std::cout << "Testing parseTitle\n";
    std::string title = parseTitle(filename);
    std::cout << "Input:\t" << filename << std::endl;
    std::cout << "Output:\t" << title << std::endl << std::endl;
}
void testParsingFunctions(std::string filename)
{
    testParseCast(filename);
    testParseTitle(filename); 
}
    */
void testStrRem(std::string &s, char d, bool b)
{
    std::cout << "Testing removal - removing everything ";
    if (b)  {   std::cout << "before ";  }
    else    {   std::cout << "after ";   }
    std::cout << d << "\nInput:\t" << s << std::endl;
    removeStr(s, d, b);
    std::cout << "Output:\t" << s << std::endl << std::endl;
}

/*
std::vector<filepath_t> readFilelistFromText(std::string path)
{
    std::ifstream inFile;
    inFile.open(path, std::ios::in);
    std::vector<filepath_t> files(0);
    if (!inFile){std::cerr << "Couldn't open specified text file for reading" << std::endl;}    
    else{
        for (std::string line; std::getline(inFile, line);){
            filepath_t f;
            f.path = path;
            f.name = line;
            files.push_back(f);
        }   
    }
    return files;
}
*/
// test main
void testTabs(void)
{
    for (int l = 1; l <= 30; l++)
    {
        std::string line("");
        for (int i = 1; i <= l; i++)
        {
            line += std::to_string(l%10);
        } 
        line += "\ttab";
        std::cout << line << std::endl;
    }
}
void runTests(){
    /*
    // Test Date Methods
    testDates("September 30, 2013");
    testDates("April 4th, 1979");
    testDates("Jan. 7, 1988");
    testDates("Jul 3rd 1988");
    testDates("Nov 30 2010");
    testDates("January 2nd, 1980");
    testTodaysDate();
    testAge("Nov 30 2010", "January 2nd, 1980");
    
    // Test Parsing Methods
    testBasename("/usr/local/libavcodec/avutil.sh");
    std::string testLink("<div class=photo><a href=\"http://www.sourceurl.ca/lib/avcodec\"></div>");
    testRemoveString(testLink, '=', true);
    testRemoveString2(testLink, "href=\"", true);
    testRemoveString(testLink, '\"', false);
    testRemoveString2(testLink, "\"", false);
    testHeight("6'2\"");

    // Filename Parsing Methods
//    std::string testTitle("Jodi Lee, Emma Mae & Evelyn - Why Not? feat. Gemma Massey (2012, 1080p).mp4");
//    testParsingFunctions(testTitle);
//    std::string testStr("Jodi Lee, Emma Mae & Evelyn - Why Not? feat. Gemma Massey.mp4");
//  testStrRem(testStr, '.', false);
//  testStrRem(testStr, '-', true);
//    testParsingFunctions(testStr);
    */
    // cURL & HTML Parsing Methods.
   // testIAFDquery("Ariana Marie");
    testSqlString("'ariana's massage bustin''");
}
