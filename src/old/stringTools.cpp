#include <algorithm>
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <ctype.h>
#include <iostream>
#include <sys/types.h>
#include <string>
#include <vector>
#include "Filepath.h"   
#include "Scene.h"
#include "lists.h"
#include "output.h"
#include "stringTools.h"
#include "structs.h"
#include <QString>
/*
bool isNum(std::string str)
    {
        bool valid_numString = true;
        for (size_t i = 0; i < str.size(); ++i)
        {
            if (legal_digits.find(str.at(i)) == std::string::npos)  // if we DON'T find the current character in the list of our only allowed characters, this string is illegal.
            {
                valid_numString = false;
                std::cerr << "Error: Attempting to create Date from non-numerical String: " << str.at(i) << std::endl;
            }
        }
        return valid_numString;
    }
    */
bool emp(std::string s) {  return s.empty();    }
bool emp(QString s)     {  return s.isEmpty();  }
bool legalString(std::string test, std::string legalChars)
{
    bool legal = true;  // assume true.
    for (size_t i = 0; i < test.size() && legal; ++i)
    {
        if (legalChars.find(test.at(i)) == std::string::npos)
        {
            //  If we can't find the current character from the string under test inside the list of legal characters,
            //  then the character is illegal, and thus the string is illegal.
            legal = false;
            std::cerr << "Illegal Character found in \"" << test << "\": \'" << test.at(i) << "\'" << std::endl;
        }
    }
    return legal;
}
bool legalDateString(std::string test)
{
    return legalString(test, "0123456789-.");
}
bool legalNumberString(std::string test){
    return legalString(test, "0123456789");
}
void illegalCharMsg(std::string str)
{
    std::cerr << "\nError: Couldn't Convert \"" << str << "\" to date because it contains illegal characters\n\n";
}
bool contains(std::string bigstr, std::string smallstr)
{
    return (bigstr.find(smallstr) != std::string::npos);
}
std::string removeBefore(std::string o, char c, int times)
{
    for (int i = 0; i < times; i++) {removeStr(o, c, true);}
    return o;
}
std::string removeBefore(std::string o, std::string c, int times)
{
    for (int i = 0; i < times; i++) {removeStr(o, c, true);}
    return o;
}
std::string removeAfter(std::string o, char c, int times)
{
    for (int i = 0; i < times; i++) {removeStr(o, c, false);}
    return o;
}
std::string removeAfter(std::string o, std::string c, int times)
{
    for (int i = 0; i < times; i++) {removeStr(o, c, false);}
    return o;
}
std::string getBetween(std::string o, char pre, char post)
{
    if (o.find(pre) != std::string::npos)
    {
	removeStr(o, pre, true);
	removeStr(o, post, false);
    }
    else {
	o = "";
    }
    return o;
}
std::string getBetween(std::string o, std::string pre, std::string post)
{
    if (o.find(pre) != std::string::npos)
    {
	removeStr(o, pre, true);
	removeStr(o, post, false);
    }
    else {
	o = "";
    }
    return o;
}
std::string getTabString(std::string s, int maxTabs)
{
    std::string tabsString = "";
    int tabs = maxTabs - (s.size()/8);
    if (tabs > -1)
    {
        for(int i = 0; i <= tabs; i++)
        {
            tabsString += "\t";
        }
    }
    return tabsString;
}

std::string addTabs(std::string s, int maxTabs)
{
    int tabCount = maxTabs - (s.size()/8);
    for (int tabNum = 0; tabNum < tabCount; ++tabNum)
        s.push_back('\t');
    return s;
}

std::string replaceString   (std::string full, std::string swapOut, std::string swapIn)
{
    boost::algorithm::replace_all(full, swapOut, swapIn);
    return full;
}

 
std::string properName(std::string s)
{
    std::string n("");
    if(s.size() > 1)
    {
        try
        {
            removeStr(s,  '(');
            removeStr(s, '-');  
            bool capitalize = true;
            for (std::string::iterator i = s.begin(); i != s.end(); ++i)
            {
                if (isalpha (*i))
                {
                    if(capitalize)
                    {
                        capitalize = false;
                        n.push_back(toupper(*i));
                    }
                    else
                    {
                        n.push_back(tolower(*i));
                    }
                }
                else if (*i == ' ' || *i == '.')
                {
                    capitalize = true;
                    n.push_back(*i);
                }
            }
        }catch (std::exception &e){
            logError(e.what());
        }
        trimWhitespace(n);
    }
    return n;
}
// Like 'properName', but allows for non-alphabetic characters.
std::string toTitle(std::string s)
{
    std::string title("");
    bool capital = true;
    if (!s.empty())
    {
        for (std::string::iterator i = s.begin(); i != s.end(); ++i)
        {
            try
            {
                if (isalpha(*i)){
                    if (capital){
                        capital = false;
                        title.push_back(toupper(*i));
                    }else{
                        title.push_back(*i);
                    }
                }else if (*i == ' '){
                   capital = true;
                    title.push_back(*i);
                }else{
                   title.push_back(*i);
               }
            }catch(std::out_of_range &e){logError(e.what());}
        } // for
    }   // if
    return title;
}

bool isNumber(std::string s)
{
    bool numerical = true;
    for (std::string::iterator c = begin(s); c!= end(s) && numerical; ++c)
    {
        if (!isdigit(*c))   {   numerical = false;  }
    }
    return numerical;
}


std::string unixSafe(std::string s)
{
    std::string n = "";
    //std::string delimitees = " &[](){}\"\'";
    for (std::string::iterator I = s.begin(); I != s.end(); ++I)
    {
    	//if (delimitees.find(*I) != std::string::npos)
    	if (*I==' '||*I=='&'||*I=='['||*I==']'||*I=='('|| *I==')'||*I=='\\'||*I=='\"'||*I=='\''||*I==',')
            n.push_back('\\');
    	n.push_back(*I);
    }
    return n;
}

QString sqlSafe(std::string s)
{
    QString safeString("");
    if (!s.empty())
    {
        safeString.append("'");
        int offset = 0;
        int rear_offset = 0;
       try{
            if (s.at(0) == '\'')
            {
                offset = 1;
            }
            if (s.at(s.size() - 1) == '\'')
            {
                rear_offset = 1;
            }
            for (std::string::iterator c = s.begin() + offset; c != s.end() - rear_offset; c++)
            {
                if (*c == '\'')
                     safeString.append("''");
                else if (*c != '\"' && *c != ';')
                    safeString.append(*c);
            }
            while(safeString.at(safeString.size() - 1) == '\'')
            {
                safeString.resize(safeString.size() - 1);
            }
        }
        catch(std::exception &e)
        {
            logError(e.what());
        }
        safeString.append("'");
    }
    return safeString;
}

QString sqlSafe(int i)          {   return QString("%1").arg(i);    }
QString sqlSafe(double d)       {   return QString("%1").arg(d);    }

//-------------------------------------------------------------------
//  Trim the whitespace from the start & end of a std::string.
//-------------------------------------------------------------------
void trimWhitespace(std::string &oStr){

    try{
        int lastPos = oStr.size() - 1;
        char last = oStr[lastPos];
        // Erase whitespace from the end of the std::string.
        while (last == ' '  || last == '\t' || last == '\n' || last == '\r' )
        {
            oStr.resize(lastPos);
            lastPos = oStr.size() - 1;
            last = oStr[lastPos];
        }
        // Erase whitespace from the beginning of the std::string
        while(oStr[0] == ' ' || oStr[0] == '\t' || oStr[0] == '\n' || oStr[0] == '\r')
        {
            oStr.erase(0, 1);
        }
    }catch(std::out_of_range &e){
        logError(e.what());
    }
}
// trim whitespace and convert the string to lowercase
std::string stripString (std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    trimWhitespace(s);
    return s;
}
std::string getBasename(std::string path)
{
    size_t lastSlash = path.rfind('/');
    std::string basename = path.substr(lastSlash+1);
    return basename;
}
std::string getExtension(std::string name)
{
    size_t oldLength = name.size();
    removeStr(name, '.', true);
    if (name.size() < oldLength)
        return name;
    else
        return "";
}
// remove every instance of a specified character from the given string
void removeChar(std::string &s, char c)
{
    if (!s.empty())
    {
        std::string n = "";
        for (std::string::iterator i = begin(s); i != end(s); ++i)
        {
            if (*i != c)
                n.push_back(*i);
        }
        s = n;
    }
}
// remove a specified sub-string and everything after OR before it from the string, s.
bool removeStr(std::string &s, std::string c, bool removeBefore)
{
    bool found = false;
    size_t pos;
    try
    {
        if ((pos = s.find(c)) != std::string::npos)
        {
            found = true;
            if (removeBefore)
            {
                std::string temp(s.begin() + pos + c.size(), s.end());
                s = temp;
            }
            else
            {
                std::string temp(s.begin(), s.begin() + pos);
                s = temp;
            }
        }
    }catch(std::out_of_range &e){
        std::cerr << __FUNCTION__  << " @ " << __LINE__ << ": " << e.what() << std::endl;
    }
    return found;
}
// remove a specified character and everything after OR before it from string s.
bool removeStr(std::string &s, char c, bool preChar){
    bool found = false;
    if (!s.empty())
    {
        try{

            if(preChar)
            {    // Remove from beginning of string
            	for (size_t i = 0; i < s.size() && !found; i++){
            	    if(s[i] == c){
                		found = true;
                		s.erase(0, ++i);
            	    }
            	}
            }
            else
            {	    // Remove from end of string
            	for (size_t i = s.size(); i > 0 && !found; --i){
            	    if (s[i] == c){
                		found = true;
                		s.resize(i);
            	    }
            	}
            }
        }catch(std::out_of_range &e){
            logError(e.what());
        }
        trimWhitespace(s);
    }
    return found;
}


// given a string, generate a filename from it. assumes a jpeg extension if not passed one.`
std::string toImage(std::string name, std::string ext)
{
    std::string n("");
    if (!name.empty())
    {
	// list of characters that will be converted to underscores.
	try{

	    std::transform(name.begin(), name.end(), name.begin(), ::tolower); 
	    std::string illegalChars(",'    ");
	    for (std::string::iterator i = begin(name); i != end(name); ++i)
	    {
		if (illegalChars.find(*i) != std::string::npos)
		    n.push_back('_');
		else
		    n.push_back(*i);
	    }
	    n += "." + ext;
	}catch(std::exception &e){ logError(e.what());}
    }
    return n;
}

std::string toImageFormatString(std::string name, std::string ext)
{
    std::string n("");
    if (!name.empty())
    {
    // list of characters that will be converted to underscores.
    try{

        std::transform(name.begin(), name.end(), name.begin(), ::tolower); 
        std::string illegalChars(",'    ");
        for (std::string::iterator i = begin(name); i != end(name); ++i)
        {
            if (illegalChars.find(*i) != std::string::npos)
                n.push_back('_');
            else
                n.push_back(*i);
        }
        n += "\%02d." + ext;
        }catch(std::exception &e){ logError(e.what());}
    }
    return n;
}
std::vector<std::string> tokenizeString(std::string s, char delimiter)
{
    std::vector<std::string> tokens(0);
    size_t pos = 0;
    std::string remaining = s;
    if (s.find(delimiter) != std::string::npos)
    {
        try{
        	while((pos = remaining.find(delimiter)) != std::string::npos)
        	{
        	    std::string temp = remaining.substr(0, pos);
        	    trimWhitespace(temp);
        	    if (!temp.empty())
        		  tokens.push_back(temp);
        	    remaining.erase(0, pos + 1);
        	}
        	size_t end_pos = s.rfind(delimiter);
        	if (end_pos != std::string::npos)
        	{   
        	    std::string lastTok = s.substr(end_pos + 1, s.size());
        	    trimWhitespace(lastTok);
        	    if (!lastTok.empty())
        		  tokens.push_back(lastTok);
        	}
        }catch(std::out_of_range &e){
            logError(e.what());
        }
    }
    else{
	   tokens.push_back(s);
    }
    return tokens;
}


std::vector<std::string> tokenizeString(std::string s, std::string delimiter)
{
    std::vector<std::string> tokens(0);
    size_t pos = 0;
    std::string remaining = s;
    try{
        while((pos = remaining.find(delimiter)) != std::string::npos)
        {
            std::string temp = remaining.substr(0, pos);
            trimWhitespace(temp);
	    if (!temp.empty())
	    {
                tokens.push_back(temp);
            }
            remaining.erase(0, pos + delimiter.size());
        }
        size_t end_pos = s.rfind(delimiter);
        if (end_pos != std::string::npos)
        {   
            std::string lastTok = s.substr(end_pos + delimiter.size(), s.size());
            trimWhitespace(lastTok);
            if (!lastTok.empty())
            {
    	       tokens.push_back(lastTok);
            }
        }
    }catch(std::out_of_range &e){
        logError(e.what());
    }
    return tokens;
}

std::vector<std::string> recursiveTokenizer (std::string s, std::vector<std::string> delimList, int idx)
{
    std::vector<std::string>allTokens(0);      
    std::vector<std::string>newTokens = tokenizeString(s, delimList.at(idx));    // tokenize the passed string.
        // Base Case
    if (idx == delimList.size() - 1)
    {           // if we used the final delimiter, add all the tokens to the vector we pass back.
       allTokens.insert(allTokens.end(), newTokens.begin(), newTokens.end());
    }else{  // Rec. Case                
        for(int i = 0; i < newTokens.size(); i++)
        {       // if more delimiters exist, pass each token recursively with the index of the next delimiter.
            std::vector<std::string>subTokens = recursiveTokenizer(newTokens.at(i), delimList, idx+1);
                // a vector of sub-tokens will be returned for each recursive call that we add to the end of the list we'll pass back.
            allTokens.insert(allTokens.end(), subTokens.begin(), subTokens.end());
        }
    }
    return allTokens;   
}

std::vector<std::string> recursiveTokenizer(std::string s, std::string delimList, int idx)
{
    std::vector<std::string>allTokens(0);		
    std::vector<std::string>myTokens = tokenizeString(s, delimList.at(idx));	// tokenize the passed string.
	    // Base Case
    if (idx == delimList.size() - 1)
    {		    // if we used the final delimiter, add all the tokens to the vector we pass back.
	   allTokens.insert(allTokens.end(), myTokens.begin(), myTokens.end());
    }else{  // Rec. Case				
    	for(int i = 0; i < myTokens.size(); i++)
    	{	    // if more delimiters exist, pass each token recursively with the index of the next delimiter.
    	    std::vector<std::string>subTokens = recursiveTokenizer(myTokens.at(i), delimList, idx+1);
    		    // a vector of sub-tokens will be returned for each recursive call that we add to the end of the list we'll pass back.
    	    allTokens.insert(allTokens.end(), subTokens.begin(), subTokens.end());
    	}
    }
    return allTokens;
}

