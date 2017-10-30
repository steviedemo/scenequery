#include "boost/regex.hpp"
#include <iostream>
#include <string>
#include <vector>

#include "Date.h"
#include "Filepath.h"   
#include "Scene.h"
#include "stringTools.h"
#include "structs.h"

#define BUFFSIZE 4096


height_t stringToHeight(std::string s)
{
    boost::regex rx1("(\\d)'[\\s]?([\\d]{0,2})\"");
    boost::smatch hmat;
    height_t height;
    if (boost::regex_search(s, hmat, rx1))
    {
        height.feet = atoi((hmat[1].str()).c_str());
        height.inches = atoi((hmat[2].str()).c_str());
    }
    return height;
}
std::string heightToString(height_t h)
{
	std::string height = std::to_string(h.feet) + "'" + std::to_string(h.inches) + "\"";
	return height;
}

std::string toWords(height_t h)
{
    return std::to_string(h.feet) + " feet " + std::to_string(h.inches) + " inches";
}
height_t wordsToHeight(std::string s)
{
    boost::regex heightRx("(\\d)[\\s]*feet[\\s,]*(\\d)?(inches)?");
    boost::smatch hMat;
    height_t height;
    if (boost::regex_search(s, hMat, heightRx))
    {
        height.feet = atoi(hMat[1].str().c_str());
        if (hMat[2] != NULL)
            height.inches = atoi(hMat[2].str().c_str());
        else
            height.inches = 0;
    }
    return height;
}

bool strCountSort(string_count_t a, string_count_t b)
{
    if (a.getCount() > b.getCount())
    {
        return true;
    }
    else
        return false;
}
bool updateStringCounts(std::string s, std::vector<string_count_t> &l)
{
    bool found = false;
    for (int i = 0; i < (int)l.size() && !found; i++)   // go through the list looking for the item.
    {
        if (s == (l.at(i)).name)
        {
            (l.at(i)).increment();  // if you find it, increment the number of times it's been found.
            found = true;
        }
    }
    if (!found) // if the item wasn't in the list, add it.
    {
        string_count_t item;
        item.name = s;
        item.increment();
        l.push_back(item);
    }
    return !found;
}

std::vector<string_count_t> getCompanyList(std::vector<Scene> &list)
{
    std::vector<string_count_t> l(0);
    for (int i = 0; i < list.size(); i++)
    {
        std::string studio = list.at(i).getCompany();
        if (!studio.empty())
        {
            updateStringCounts(studio, l);
        }
    }
    return l;
}

void printStringCountList(std::vector<string_count_t> &l)
{
    std::cout << "\nCompany List:\n\n";
    for (int i = 0; i < (int)l.size(); i++)
    {
        string_count_t item = l.at(i);
        std::string output = item.name + ":";
        std::string tabs = getTabString(output, 2);
        std::cout << output << tabs << item.getCount() << std::endl;
    }
    std::cout << std::endl;
}



std::string sqlSafe(Filepath f)	{	return std::string("'" + f.fullpath() + "'");	}
std::string sqlSafe(rating_t r)		{	return std::string("'" + r.toString() + "'");	}

