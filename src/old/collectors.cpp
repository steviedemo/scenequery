#include <boost/regex.hpp>
#include <pqxx/pqxx>
#include <string>
#include <fstream>
#include <omp.h>
#include <vector>
#include "Scene.h"
#include "Actor-old.h"
#include "curlRequests.h"
#include "database.h"
#include "lists.h"
#include "output.h"
#include "Date.h"
#include "Filepath.h"   
#include "structs.h"
#include "stringTools.h"
#include "tools.h"


void removeScenesWithoutAddedDate(std::vector<Scene> &list)
{
    for (std::vector<Scene>::iterator S = begin(list); S != end(list); ++S)
    {
        if (!S->hasDateAdded())
            list.erase(S);
    }
}

void removeScenesWithoutReleaseDate(std::vector<Scene> &list)
{
    for (std::vector<Scene>::iterator S = begin(list); S != end(list); ++S)
    {
        if (!S->hasReleaseDate())
            list.erase(S);
    }   
}

std::vector<Scene> getScenesAddedOn(Date d)
{
    std::vector<Scene> list(0), fullList(0);// = loadScenesWhere(query);
    loadScenes(fullList);
    omp_set_dynamic(0);
    #pragma omp parallel for num_threads(32)
    for(size_t i = 0; i < fullList.size(); ++i)
    {
	if (fullList.at(i).getDateAdded().on(d))
	{
	    #pragma omp critical
	    list.push_back(fullList.at(i));
	}
    }
    std::cout << list.size() << " Scenes were added on " << d.ymd('-') << std::endl;
    return list;
}

std::vector<Scene> getScenesAddedBefore(Date d)
{
    std::string query("SELECT * FROM scenes WHERE added < " + sqlSafe(d) + ";");
    std::vector<Scene> list = loadScenesWhere(query);
    removeScenesWithoutAddedDate(list);
    return list;
}
std::vector<Scene> getScenesAddedAfter(Date d)
{
    std::string query("SELECT * FROM scenes WHERE added > " + sqlSafe(d) + ";");
    std::vector<Scene> list = loadScenesWhere(query);
    removeScenesWithoutAddedDate(list);
    return list;
}
std::vector<Scene> getScenesReleasedOn(Date d)
{
    std::string query("SELECT * FROM scenes WHERE created::date = " + sqlSafe(d) + ";");
    std::vector<Scene> list = loadScenesWhere(query);
    return list;
}
std::vector<Scene> getScenesReleasedBefore(Date d)
{
    std::string query("SELECT * FROM scenes WHERE created< " + sqlSafe(d) + ";");
    std::vector<Scene> list = loadScenesWhere(query);
    removeScenesWithoutReleaseDate(list);
    return list;
}
std::vector<Scene> getScenesReleasedAfter(Date d)
{
    std::string query("SELECT * FROM scenes WHERE created > " + sqlSafe(d) + ";");
    std::vector<Scene> list = loadScenesWhere(query);
    removeScenesWithoutReleaseDate(list);
    return list;
}

void listScenesAddedOn(Date d, std::string name)          {   toTextFile(getScenesAddedOn(d), name);         }
void listScenesAddedBefore(Date d, std::string name)      {   toTextFile(getScenesAddedBefore(d), name);     }
void listScenesAddedAfter(Date d, std::string name)       {   toTextFile(getScenesAddedAfter(d), name);      }
void listScenesReleasedOn(Date d, std::string name)       {   toTextFile(getScenesReleasedOn(d), name);      }
void listScenesReleasedBefore(Date d, std::string name)   {   toTextFile(getScenesReleasedBefore(d), name);  }
void listScenesReleasedAfter(Date d, std::string name)    {   toTextFile(getScenesReleasedAfter(d), name);   }
std::vector<Scene> topScenes (std::vector<Scene> &s, int n)
{
    std::sort(s.begin(), s.end(), byRating);
    std::vector<Scene> list(s.begin(), s.begin() + n);
    return list;
}


void toUpdate(std::vector<Scene> &scenes, bool newFiles, int cutoffQuality, int cutoffSize, bool ratedOnly)
{
    std::cout << "\nGenerating List of Updatable Files, considering everything under "<< cutoffSize << "MB or lower than " << cutoffQuality << "p as updatable.\n" << std::endl;
    std::fstream sml, na, bz, ea, other;
    std::vector<Scene> n(0), b(0), e(0), m(0), o(0), a(0);
    for (std::vector<Scene>::iterator s = begin(scenes); s != end(scenes); ++s)
    {
        try{
            if (!ratedOnly || (ratedOnly && s->rated()))
            {
        	    if ((s->getSize() < 150 || s->getHeight() < 480) && s->getHeight() > -1){
        			//m.push_back(filepath_t(s->filepath(), s->filename()));
        		      m.push_back(*s);
                }
    		    else if (s->getHeight() < cutoffQuality && !s->getCompany().empty())
    		    {
        			if (!s->hasReleaseDate() || s->getYear() > 2008)
        			{
        			    std::string com = s->getCompany();
        			    if (com == "Naughty America"){
        		      		//n.push_back(filepath_t(s->filepath(), s->filename()));
                            n.push_back(*s);                      
        			    }
        			    else if (com.find("Brazzers") != std::string::npos){
            				//b.push_back(filepath_t(s->filepath(), s->filename()));
        			         b.push_back(*s);
                        }
        			    else if (com.find("Evil") != std::string::npos){
                            //e.push_back(filepath_t(s->filepath(), s->filename()));
            				e.push_back(*s);
        			    }
        			    else{
                            //o.push_back(filepath_t(s->filepath(), s->filename()));
                            o.push_back(*s);
        			    }
        			}
    	        }
    	    }
        }catch(std::out_of_range &e){logError(e.what());}
    }
    a.insert(a.end(), n.begin(), n.end());
    a.insert(a.end(), b.begin(), b.end());
    a.insert(a.end(), e.begin(), e.end());
    toTextFile(n, "./data/NA_low_res.txt");
    toTextFile(b, "./data/BZ_low_res.txt");
    toTextFile(e, "./data/EA_low_res.txt");
    toTextFile(m, "./data/Small_files.txt");
    toTextFile(o, "./data/Other_low_Res.txt");
    toTextFile(a, "./Composite.txt");
    // Create a single file composed from all the entries, sorted by company, then by directory.
    size_t total = n.size() + b.size() + e.size() + o.size() + m.size();
    std::cout << "Updatable Files Found:\n";
    std::cout << "\tBrazzers:\t" << b.size() << "\n\tEvil Angel:\t" << e.size() << "\n\tNA:\t\t" << n.size() << std::endl;
    std::cout << "\tOther Company:\t" << o.size() << "\n\tNon-Company:\t" << m.size() << "\n\tTotal:\t" << total << std::endl << std::endl; 
}
 
void	totalSceneList_byCompany(std::string name)
{
	std::cout << "Getting filmography for " << name << std::endl;
	std::vector<Scene> temp = getOnlineFilmography(getIAFDhtml(getIAFDURL(name)));
	std::vector<string_count_t> list = getCompanyList(temp);
	printStringCountList(list);
}


std::vector<Scene> getScenesWithNoReleaseDate(void)
{
    return loadScenesWhere("select * from scenes where created = '' or created = null;");
}