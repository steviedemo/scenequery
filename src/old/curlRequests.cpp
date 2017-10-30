#include <curl/curl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include <ctype.h>
#include <boost/regex.hpp>
#include "curlRequests.h"
#include "output.h"
#include "stringTools.h"
#include "systemInterface.h"
#include "Scene.h"

#define PHOTO_FOLDER "./photos"
#define MEASUREMENTS "<dt>Measurements:</dt>"
#define BUFFSIZE     4096
//--------------------------------------------
// LOWEST LEVEL ROUTINES
//--------------------------------------------


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

//----------------------------------------------
//		FREEONES
//----------------------------------------------

std::string fetchHTML(std::string url)
{
	CURL *curl;
	CURLcode res;
	std::string readBuffer;

	curl = curl_easy_init();
	if (curl){
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	return readBuffer;
}

std::string getData(std::string data, const char *title)
{
	//std::string titleHTML("<dt>" + std::string(title) + "</dt>");
	boost::smatch match;
	std::string matchData("");
	boost::regex data_rx("<dt>" + std::string(title) + "</dt>" + "\n<dd>([^\n]+)</dd>");
	if (regex_search(data, match, data_rx))
	{
		if (std::string(match[1]).find("Unknown") == std::string::npos)
		{
			matchData = match[1].str();
		}
	}
	return matchData;
}

/* Function:	getFreeonesURL
 * Expects:		non-empty name.
 * Returns:		Expected web address of performer page given the name provided
 */
std::string getFreeonesURL(std::string name)
{
	std::string queryString("http://www.freeones.ca/html/");
	queryString.push_back(tolower(name.at(0)));
	queryString += std::string("_links/");
	for(int i = 0; i < (int)name.size(); i++)
	{
		if (name.at(i) == ' ') 		{	queryString.push_back('_');			}
		else if (name.at(i) != '.')	{	queryString.push_back(name.at(i));	}	
	}
	return queryString;
}

/* Function: 	fetchFreeonesBio
 * Expects:		non-empty html string, pointer to a bio struct.
 */
bool fetchFreeonesBio(std::string htmlString, bio_t &bio){
	bool found = false;
	if (!htmlString.empty())
	{
		size_t start = htmlString.find("<dt>Babe Name:");
		std::string partialHTML(htmlString.begin() + start, htmlString.end());
		size_t endBio = partialHTML.find("</div>");
		//std::cout << "Start: " << start << "\t" << "End: " << endBio << "\tTotal Size: " << htmlString.size() << std::endl;
		
		if (start == std::string::npos || endBio == std::string::npos)
		{
			return false;
		}
		std::string bioHTML(partialHTML.begin(), partialHTML.begin() + endBio);
		//std::cout << bioHTML << std::endl;
		bio.measurements = getData(bioHTML, "Measurements:");
		bio.name = getData(bioHTML, "Babe Name: ");
		bio.aliases = getData(bioHTML, "Aliases:");
		bio.city = getData(bioHTML, "Place of Birth:");
		bio.nationality = getData(bioHTML, "Country of Origin:");
		bio.eyeColor =  getData(bioHTML, "Eye Color:");
		bio.hairColor = getData(bioHTML, "Hair Color:");
		bio.piercings = getData(bioHTML, "Piercings:");
		bio.tattoos = getData(bioHTML, "Tattoos:");
		std::string bday = getData(bioHTML, "Date of Birth:");
		removeStr(bday, '(', false);
		bio.addBirthday(bday);
	}
	else{
		std::cerr << __FUNCTION__ << " @ " << __LINE__ << ": No HTML data in provided string" << std::endl;
	}
	return found;
}

//----------------------------------------------
//		IAFD
//----------------------------------------------

std::string getIAFDhtml(std::string url)
{
	return systemCall("curl " + url);
}

/* Function:	getIAFDURL
 * Expects:		non-empty name.
 * Returns:		Expected web address of performer page given the name provided
 */
std::string getIAFDURL(std::string name)
{
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	std::string variant1(""), variant2("");
	for (int i= 0; i < (int)name.size(); i++)
	{
		if (name.at(i) == ' ')
			variant2.push_back('-');
		else{
			variant1.push_back(name.at(i));
			variant2.push_back(name.at(i));
		}
	}
	std::string url("http://www.iafd.com/person.rme/perfid=" + variant1 + "/gender=f/" + variant2 + ".htm"); 
	return url;
}
/* Function:	getIAFDBioLine
 * Expects:		non-empty html code in string.
 * Returns:		Line containing biographical data of performer.
 */
std::string getIAFDBioLine(std::vector<std::string> &html)
{
	std::string line("");
	if (!html.empty())
	{
		std::vector<std::string> lines(0);
		// Get data from side column
		getSelectLines(html, lines, "<p class=\"bioheading\">Birthday");
		if (lines.size() == 1)
		{
			line = lines.at(0);
		}
		// Ge Data from main section
		bool addData = false; 	// state-machine style flag to determine when we're in a section of HTML that we are saving.
		bool keepGoing = true;	// loop terminator in case of a found bio section (hopefully always)
        for (size_t i = 0; i < html.size() && keepGoing; i++)
		{
			if (addData)	// if we're in the bio section
			{
				if (html.at(i).find("<p><b>Find where <a style=") != std::string::npos) // check if this is the end of the bio section
				{
					keepGoing = false;	// terminate the loop.
				}
				else	// If we're still in the Bio Section, add the line to the string to be returned.
				{
					line += html.at(i);
				}
			}
			else if (html.at(i).find("<p class=\"bioheading\">Ethnicity") != std::string::npos)
			{	// look for beginning of bio section.
				addData = true;
			}
		}
	//	if (addData) // isn't reset to false, so a true value here tells us there was data gathered.
	}
	return line;
}

std::string getBioLineRx(std::string heading)
{
	std::string rx = "<p class=\"bioheading\">" + heading + "</p><p class=\"biodata\">([a-zA-z0-9/()\\s]+)</p>";
	return rx;
}

bool fetchIAFDBio(std::vector<std::string> &html, bio_t &bio)
{
	bool success = true;
	std::string line = getIAFDBioLine(html);
	boost::regex activeYearsRx("class=\"biodata\">([\\d]{4})-([\\d]{4}) \\(Started");
	boost::regex ethRx(getBioLineRx("Ethnicity"));
	boost::regex heightRx(getBioLineRx("Height"));
	boost::regex weightRx(getBioLineRx("Weight"));
	boost::regex measurementsRx(getBioLineRx("Measurements"));
	boost::regex tattoosRx(getBioLineRx("Tattoos")); 
	boost::regex piercingsRx(getBioLineRx("Piercings"));
	boost::regex hairRx(getBioLineRx("Hair Color"));
	boost::regex natRx(getBioLineRx("Nationality"));
	boost::smatch act, eth, hei, wei, mea, tat, pie, hai, nat;
	if (boost::regex_search(line, act, activeYearsRx)){	
		bio.addCareer(atoi(act[1].str().c_str()), atoi(act[2].str().c_str()));
	}
	if (boost::regex_search(line, eth, ethRx))			{	bio.ethnicity = eth[1].str();}
	if (boost::regex_search(line, hei, heightRx))		{	bio.height = wordsToHeight(hei[1].str());}
	if (boost::regex_search(line, wei, weightRx))		{	bio.weight = atoi(wei[1].str().c_str());}
	if (boost::regex_search(line, mea, measurementsRx))	{	bio.measurements = mea[1].str();		}
	if (boost::regex_search(line, tat, tattoosRx))		{	bio.tattoos = tat[1].str();				}
	if (boost::regex_search(line, pie, piercingsRx))	{	bio.piercings = pie[1].str();			}
	if (boost::regex_search(line, hai, hairRx))			{	bio.hairColor = hai[1].str();			}
	if (boost::regex_search(line, nat, natRx))			{	bio.nationality = nat[1].str();			}
	return success;
}

bool fetchIAFDBio(std::string html, bio_t &bio)
{
	std::vector<std::string> htmlvec = tokenizeString(html, '\n');
	return fetchIAFDBio(htmlvec, bio);
}

bool fetchHeightAndWeight(std::string name, height_t &h, int &weight)
{
	bool found = false;
	
	std::string html = getIAFDhtml(getIAFDURL(name));
	if (!html.empty())
	{
		std::vector<std::string>lines = tokenizeString(html, '\n');
		for (std::vector<std::string>::iterator l = begin(lines); l != end(lines) && !found; ++l)
		{
			if (contains(*l, "<p class=\"bioheading\">Weight</p>"))
			{

				std::cout << *l << std::endl;
				
				std::string temp = *l;
				removeStr(temp, "Weight</p><p class=\"biodata\">", true);
				removeStr(temp, "lbs", false);
				if (!temp.empty())
					weight = stoi(temp);
				
				temp = *l;
				removeStr(temp, "(", true);
				removeStr(temp, "cm)", false);
				if (!temp.empty())
					h.set(stoi(temp));
				found = true;
			}
		}
	}
	return found;
}


bool parseHTMLscene(std::string html, Scene &s)
{
	bool found_data = false;
	boost::regex titleRx("<a title=\"Info on the movie (.+): featuring");
	boost::regex companyRx("<a href=\\\"/distrib\\.rme/.+>(.+)</a></td><td>");
	boost::regex tagsRx("<i>(.+)</i>");
	boost::regex yearRx("year=(\\d\\d\\d\\d)");
	boost::smatch tMat, cMat, tagMat, yMat;
	if (boost::regex_search(html, tMat, titleRx)){
		s.setTitle(tMat[1].str());
		found_data = true;
	}
	if (boost::regex_search(html, cMat, companyRx)){
		s.setCompany(cMat[1].str());
		found_data = true;
	}
	if (boost::regex_search(html, yMat, yearRx)){
		s.setYear(atoi(yMat[1].str().c_str()));
		found_data = true;
	}
	if (boost::regex_search(html, tagMat, tagsRx))
	{
		std::string tags = tagMat[1].str();
		s.addTags(tokenizeString(tags, " "));
		found_data = true;
	}
	
	return found_data;
}

// Arguments: html_vec: The html for the whole page, each line being a separate element
//			  newVec:	The empty vector we fill from select elements of html_vec
// Behaviour: Check each element of html_vec and put the ones that match our criteria into newVec.
void getSelectLines(std::vector<std::string> &html_vec, std::vector<std::string> &newVec, std::string criteria)
{
	for (size_t i = 0; i < html_vec.size(); i++)
	{
		if (html_vec.at(i).find(criteria) == 0)
		{
			newVec.push_back(html_vec.at(i));
		}
	}
}

std::vector<Scene> getOnlineFilmography(std::string html){
	// Cut down the string to just the part that contains the scene list.
	std::vector<Scene> sceneList(0);
	if (!html.empty())
	{
		std::vector<std::string> htmlVector(0), htmlList(0);
		htmlVector = tokenizeString(html, "\n");	// Split HTML into vector, each line becoming a separate element.
		getSelectLines(htmlVector, htmlList, "<tr><td><a title=");
		// Each item should now start "<a title=..." or "<" and end with "</a></tr></td>"
        for (size_t i = 0; i < htmlList.size(); i++)
		{
			/*
			std::cout << "#" << i << ": " << htmlList.at(i) << "\n\n";
			*/
			Scene s;
			if (parseHTMLscene(htmlList.at(i), s)){	
				// if the regex matches the data we expect to find in the html line, 
				// it parses out the relevant data, and puts it into the passed scene object.
				if (s.notNull())
				{
					sceneList.push_back(s);
				}
			}
		}
	}
	return sceneList;
}

std::vector<Scene> getOnlineFilmography(std::vector<std::string> html){
	// Cut down the string to just the part that contains the scene list.
	std::vector<Scene> sceneList(0);
	if (html.size() > 0)
	{
		std::vector<std::string> htmlList(0);
		getSelectLines(html, htmlList, "<tr><td><a title=");
			// Each item should now start "<a title=..." or "<" and end with "</a></tr></td>"
            for (size_t i = 0; i < htmlList.size(); i++)
			{
				/*
				std::cout << "#" << i << ": " << htmlList.at(i) << "\n\n";
				*/
				Scene s;
				if (parseHTMLscene(htmlList.at(i), s)){	
					// if the regex matches the data we expect to find in the html line, 
					// it parses out the relevant data, and puts it into the passed scene object.
					if (s.notNull())
					{
						sceneList.push_back(s);
					}
				}
			}
	}
	return sceneList;
}
//------------------------------------------------------------------------------
//---------------------------_HIGH LEVEL UTILITIES------------------------------
//------------------------------------------------------------------------------

bio_t fetchFreeonesBio(std::string name)
{
	bio_t bio;
	std::string html = fetchHTML(getFreeonesURL(name));
	fetchFreeonesBio(html, bio);
	return bio;
}

bio_t fetchIAFDBio(std::string name)
{
	std::string url = getIAFDURL(name);
	std::string html = fetchHTML(url);
	bio_t newBio;
	fetchIAFDBio(html, newBio);
	return newBio;
}
bio_t fetchBio (std::string name)
{
	bio_t b1 = fetchFreeonesBio(name);
	bio_t b2 = fetchIAFDBio(name);
	// merge the two data sets.

	return b1;
}

std::vector<Scene> fetchFilmography(std::string name)
{
	std::string url = getIAFDURL(name);
	std::string html = fetchHTML(url);
	std::vector<Scene> scenes = getOnlineFilmography(html);
	return scenes;
}

