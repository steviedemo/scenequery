#include <boost/regex.hpp>
#include <pqxx/pqxx>
#include <string>
#include <fstream>
#include <omp.h>
#include <vector>
#include "Actor-old.h"
#include "Date.h"
#include "Filepath.h"
#include "Scene.h"
#include "curlRequests.h"
#include "database.h"
#include "lists.h"
#include "output.h"
#include "structs.h"
#include "stringTools.h"
#include "systemInterface.h"
#include "tools.h"

#define NUM_THREADS 24
#define BUFFSIZE 4096

std::vector<abbr_t> companyAbbreviation{
    abbr_t("Amateur Allure",        "AA"),
    abbr_t("Burning Angel",         "BA"),
    abbr_t("Bang Bros",             "BB"),
    abbr_t("Brazzers",              "BZ"),
    abbr_t("Brazzers Network",	    "BZ"),
    abbr_t("Combat Zone",           "CZ"),
    abbr_t("DDF Network",           "DDF"),
    abbr_t("Digital Playground",    "DP"),
    abbr_t("Evil Angel",            "EA"),
    abbr_t("Elegant Angel",         "ElA"),
    abbr_t("Fantasy HD",            "FHD"),
    abbr_t("Family Strokes",        "FS"),
    abbr_t("Harmony Vision",        "HV"),
    abbr_t("Jules Jordan",          "JJ"),
    abbr_t("Lethal Hardcore",       "LH"),
    abbr_t("MOFOs",                 "MF"),
    abbr_t("My XXX Pass",           "MXP"),
    abbr_t("Naughty America",       "NA"),
    abbr_t("Nubiles",               "NF"),
    abbr_t("New Sensations",        "NS"),
    abbr_t("Passion HD",	        "PHD"),
    abbr_t("Porn Pros",             "PP"),
    abbr_t("Reality Kings",         "RK"),
    abbr_t("Reality Junkies",       "RJ"),
    abbr_t("Red Light District",    "RL"),
    abbr_t("Suze Randall",          "SR"),
    abbr_t("Twisty's",              "Tw"),
    abbr_t("Vixen X",               "VX"),
    abbr_t("21 Sextury",            "21S"),
    abbr_t("21 Sextury",	        "21")

};

void updateWeights(void)
{
    try{
        pqxx::connection C(DB_CREDS);
        unsigned int idx = 0, updates = 0;
        if (!C.is_open()){  logError("Error Opening Scene Database");   }
        else{
            pqxx::nontransaction N_select(C);
            std::string sql_select("SELECT NAME FROM ACTORS WHERE WEIGHT='' OR WEIGHT=NULL;");
            pqxx::result R(N_select.exec(sql_select.c_str()));
            N_select.commit();
            omp_set_dynamic(0);
            #pragma omp parallel for num_threads(NUM_THREADS)
            for (size_t i = 0; i < R.size(); ++i)
            {
                ++idx;
                pqxx::result::const_iterator I = R.begin() + i;
                std::string name = I["name"].as<std::string>();
                int weight = 0;
                height_t height(0);
                fetchHeightAndWeight(name, height, weight);
                #pragma omp critical
                {
                    std::cout << ++idx << "/" << R.size() << ":\t" << name << ":\n\tHeight:\n" << height.toString() << "\n\tWeight:\t" << weight << std::endl;
                }
                if (weight != 0)
                {
                    ++updates;
                    std::string sql_update("UPDATE actors SET WEIGHT = " + sqlSafe(weight) + ", HEIGHT = " + sqlSafe(height.toCm()) + " WHERE NAME = " + sqlSafe(name) + ";");
                    pqxx::nontransaction N_update(C);
                    N_update.commit();
                }

            }

            C.disconnect();
            std::cout << updates << "/" << idx << " Records Updated & Stored in Database\n";
        }
    }catch (std::exception e){   logError(std::string(e.what()));    }

}
void hardResetBios(void)
{
    std::vector<Actor> a(0);
    loadActors(a);
    fetchBios(a, false);
    fetchIAFDBios(a, false);
    storeActorList(a);
}

void softResetBios(void)
{
    std::vector<Actor> a(0);
    loadActors(a);
    fetchBios(a);
    fetchIAFDBios(a);
    storeActorList(a, false);
}

void updateBios(std::vector<Actor>&actors, std::vector<Scene>&scenes)
{
    fetchBios(actors);
    fetchIAFDBios(actors);
    updateAgesInScenes(actors, scenes);
}

void fetchIAFDBios(std::vector<Actor> &actors, bool useList)
{
    std::vector<size_t>IAFDupdates(0);
    unsigned int hits = 0, misses = 0, idx = 0, toUpdateWeight = 0;
    if (useList)
    {
        for (size_t i = 0; i < actors.size(); ++i)
        {
            try{
                if (actors.at(i).getWeight() != DEFAULT_INT)
                {
                    IAFDupdates.push_back(i);
                    std::cout << "Need to Update " << actors.at(i).getName() << "'s Height & Weight" << std::endl;
                }

            }catch(std::out_of_range &e){logError(e.what());}
        }
        toUpdateWeight = IAFDupdates.size();
    }
    else
    {
        toUpdateWeight = actors.size();
    }
    std::cout << "\n***** Updating Height & Weight of " << IAFDupdates.size() << " Actors From the Web *****\n" << std::endl;
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (size_t i = 0; i < IAFDupdates.size(); ++i)
    {
        int I;
        if (useList)
            I = IAFDupdates.at(i);
        else 
            I = i;
        height_t height(0);
        int weight = DEFAULT_INT;
        fetchHeightAndWeight(actors.at(I).getName(), height, weight);
        #pragma omp critical
        {
            if (weight != DEFAULT_INT)
            {
                ++hits;
                actors.at(I).setHeight(height);
                actors.at(I).setWeight(weight);
                std::cout << ++idx << "/" << toUpdateWeight << ":\t" << actors.at(I).getName() << "\'s Bio Added" << std::endl;
            }
            else 
            {
                ++misses;
                ++idx;
            }
        }
    }
    std::cout << std::endl << IAFDupdates.size() << "/" << actors.size() << " Profiles lacked Height & Weight" << std::endl;
    std::cout << hits << "/" << IAFDupdates.size() << " Profiles had Height & Weight Added" << std::endl;
    std::cout << misses << "/" << actors.size() << " Profiles still lack Height & Weight" << std::endl;     
}

void fetchBios(std::vector<Actor>&actors, bool useList)
{
    std::cout << "\n***** Updating Actress Bios from the Web *****\n" << std::endl;
    std::vector<size_t>updates(0);
    unsigned int hits = 0, misses = 0, toUpdate = 0;
    if (useList)
    {
        for (size_t i = 0; i < actors.size(); ++i)
        {
            try{
                if (!actors.at(i).hasBio())
                {
                    updates.push_back(i);
                    std::cout << "Need to Update " << actors.at(i).getName() << "'s Bio" << std::endl;
                }
            }catch(std::out_of_range &e){logError(e.what());}
        }
        toUpdate = updates.size();
    }
    else
    {
        toUpdate = actors.size();
    }
    int idx = 0;
    omp_set_dynamic(0); // disable dynamic assignment of thread count
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (size_t i = 0; i < updates.size(); ++i)
    {
        int I;
        if (useList)
            I = updates.at(i);
        else
            I = i;

        try{
            bio_t bio = fetchBio(actors.at(I).getName());
            if (!bio.empty())
            {
                #pragma omp critical
                {
                    ++hits;
                    std::cout << ++idx << "/" << updates.size() << ":\t" << bio.name << "\'s Bio Added" << std::endl;
                    actors.at(I).setBio(bio);
                }
            }
            else
            {
                #pragma omp critical
                {
                    ++misses;
                    std::cout << ++idx << "/" << updates.size() << ":\t" << actors.at(I).getName() << "\tUnavailable" << std::endl;
                }
            }
            

        }catch(std::out_of_range &e){logError(e.what());}
    }
    std::cout << std::endl << updates.size() << "/" << actors.size() << " Profiles lacked Bios" << std::endl;
    std::cout << hits << "/" << updates.size() << " Profiles had Bios Added" << std::endl;
    std::cout << misses << "/" << actors.size() << " Profiles still lack Bios" << std::endl;  

    
}


void updateAgesInScenes(std::vector<Actor> &actors, std::vector<Scene> &scenes)
{
	std::cout << "\n\nUpdating ages in Scenes...\n\n";
    std::sort(actors.begin(), actors.end(), byName);
	for (std::vector<Scene>::iterator s = scenes.begin(); s != scenes.end(); ++s)
	{
		std::vector<std::string>cast = s->getActors();	
        try	{
            int idx = 0;
            for (std::vector<Actor>::iterator A = begin(actors); A != end(actors); ++A)
            {
                Actor *a = findActor(actors, A->getName());
                if (a->hasBirthday() && s->hasReleaseDate()){
                    s->setAgeOfActorNum(age(a->getBirthday(), s->getReleaseDate()), idx++);
                }
            }
		}catch(std::out_of_range &e)  {   logError(e.what());  }
	}	// done going through the list of scenes.
	std::cout << "Updating the ages in each actor's list of scenes..." << std::endl;
	// Update the Un-linked lists of scenes that are stored in each actor's profile.
    for (std::vector<Actor>::iterator a = begin(actors); a != end(actors); ++a)
	{
		if (a->hasBirthday()){
			a->updateScenesWithAge();
		}
	}
	std::cout << "Age update complete!" << std::endl;
}



		
std::string newFilename(Scene &s)
{
    std::string name("");
    // Add First Actor, and Hyphen to start of filename
    if (s.getCastSize() > 0)
        name += s.getActorNumber(0) + " - ";
    else
        name += "Unknown - ";
    // Add Company Abbreviation if company name is available
    if (s.getCompany() != "")
    {
        std::string company = s.getCompany();
        std::string abbrv("");
        bool found = false;
        for (size_t i = 0; i < companyAbbreviation.size() && !found; i++)
        {

            if((companyAbbreviation.at(i)).word() == company){
                abbrv = companyAbbreviation.at(i).abbr();
                found = true;
            }
        }
        if (found)
            name += '[' + abbrv + "] ";     // add abbreviation
        else
            name += '[' + company + "] ";   // add full company name
    }
    name += s.getTitle();                   // add title
    if (s.getCastSize() > 1)                // add any other actors
    {
        name += " feat. " + s.getActorNumber(1);
        if (s.getCastSize() > 2)
        {
            for (size_t i = 2; i < s.getCastSize(); i++)
            {
                name += ", " + s.getActorNumber(i); 
            }
        }
    }
    bool added_date = false;
    bool added_series = false;
    bool added_quality = false;
    bool added_tags = false;
    bool added_rating = false;
    std::string series = s.getSeries();
    if (!series.empty())    // if there is a series name, and it is not contained in the title.
    {
        name += " (";
        name += series;
        added_series = true;
    }
    if (s.hasReleaseDate())
    {
        if (added_series)
            name += ", ";
        else 
            name += " (";
        Date d = s.getReleaseDate();
        name += d.ymd('.');
        added_date = true;
    }
    if (s.getHeight() > -1)
    {
        if (added_series || added_date)
            name += ", ";
        else 
            name += " (";
        name += std::to_string(s.getHeight()) + 'p';
        added_quality = true;
    }
    if (s.numTags() > 0)
    {
        if (added_series || added_date || added_quality)
            name += ", ";
        else
            name += " (";
        name += s.getTags();
        added_tags = true;
    }
    if (s.rated())
    {
        if (added_series || added_date || added_quality || added_tags)
            name += ", ";
        else
            name += " (";
        name += s.getRating();
        added_rating = true;
    }
    if (added_series || added_date || added_quality || added_tags || added_rating)
        name += ")";
    name += "." + s.filetype();
    return name;
}


bool commitName(Scene &scene, std::string newName)
{
    bool success = false;
    std::string oldName = scene.filename();
    std::string path = scene.filepath();
    std::string output("");
    // run Unix Shell Command to rename file.
    systemCall("mv \"" + scene.fullpath()+ "\" \"" + scene.filepath() + '/' + newName + '\"');
    // run shell command to list the directory contents and check if the new filename is in the list.
    output = systemCall("ls \"" + scene.filepath() + "\" | grep \"" + newName + "\"");
    if (!output.empty())
    {
        success = true;
        std::cout << "Rename successful." << std::endl;
        std::cout << "Old Name: " << oldName << std::endl;
        std::cout << "New Name: " << newName << std::endl;      

        scene.setFilename(newName);// set the Scene object's filename to the new name
        // make sure to update the name in the database 
    	if (updateFilename(path, oldName, newName))
    	    std::cout << "Name successfully updated in Scene Database" << std::endl;
    	else
    	    std::string errstr = "Error changing name from " + oldName + " to " + newName + " in scene database!";
    }
    else {
        std::string errstr = "Error changing name from " + oldName + " to " + newName + " in scene database!";
    }
    return success;
}

bool commitName(Scene &s)
{
    std::string newname = newFilename(s);
    if (!newname.empty())
	return commitName(s, newname);
    else
	return false;
}

void updateReleaseDates(void)
{
    std::vector<Scene> list = loadScenesWhere("select * from scenes where created = '' or created = null;");
    std::vector<Scene> updated(0);
    std::cout << list.size() << " Scenes are missing a release date" << std::endl;
    unsigned int idx = 0;
    omp_set_dynamic(0);
    size_t listSize = list.size();
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (size_t i = 0; i < list.size(); ++i)
    {
        Date d;
        if (!list.at(i).hasReleaseDate() && mediaInfo(list.at(i).fullpath(), d))
        {
            #pragma omp critical
            {
                std::cout << addTabs(std::to_string(++idx) + "/" + std::to_string(listSize) + ":", 1);
                std::cout << "Updating Release Date for " << list.at(i).filename() << " to " << d.toString() << std::endl;
                list.at(i).setReleaseDate(d);
                updated.push_back(list.at(i));
            }
        }
    }
    std::cout << "Finished Updating Release Dates. Now updating database entries." << std::endl;
    storeSceneList(updated);
}
