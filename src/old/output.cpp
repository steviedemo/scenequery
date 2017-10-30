
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <boost/regex.hpp>
//#include <sstream>
#include "Actor-old.h"
#include "Scene.h"
#include "stringTools.h"
#include "output.h"
#include "Date.h"
#include "Filepath.h"   
#include "structs.h"
extern Date todays_date;

void log(std::string s){
    #ifdef TEST_OUTPUT
	std::cout << s << std::endl;
    #endif
}
void    recordError         (const char * filename, const char *function, int line, std::string e, std::string f)
{
    std::ofstream logfile;

    logfile.open(LOG_FILE, std::ios::app);
    logfile << __FILE__ << " in " << __FUNCTION__ << " @ " << __LINE__ << ": " << std::endl;
    logfile << "\tMessage:\t"<< e << std::endl;
    if (!f.empty())
    {
        logfile << "\n\tFile:\t" << f << std::endl;
        std::cerr << "\n!!! Error in " << __FILE__ << " in " << __FUNCTION__ << " @ " << __LINE__ << "\n\tError: " << e << "\n\tFile: " << f << std::endl << std::endl;
    }
    else
    {
        std::cerr << "\n!!! Error in " << __FILE__ << " in " << __FUNCTION__ << " @ " << __LINE__ << "\n\tError: " << e << std::endl << std::endl;
    }
    logfile.flush();
    logfile.close();
}

// Print tracing info in parallel sections.
void omptraceLine(const char *file, const char *func, int line){
#ifdef RUN_IN_PARALLEL
#pragma omp critical
    {
	std::cout << file << " in " << func << " @ " << line << std::endl;
    }
#else
    std::cout << file << " in " << func << " @ " << line << std::endl;
#endif
}

// Print debugging message in parallel sections.
void ompDebug(const char * file, const char * func, int line, std::string message)
{
#ifdef RUN_IN_PARALLEL
#pragma omp critical
    {
	std::cout << file << " in " << func << " @ " << line << ": " << message << std::endl;
    }
#else
	std::cout << file << " in " << func << " @ " << line << ": " << message << std::endl;
#endif
}

void printUsage(void)
{
	std::cout << "Usage:\tsceneQuery <path to directory> \n\t[-s | --silent]\n\t[-n | --non-recursive] \n\t[-t | --test] " << std::endl;
    std::cout << "\t[-f | --find <query>]\n\t[-fa | --find-actor <actorname>]\n\t[-ga | --get-ages]" << std::endl;
    std::cout << "\t[-lts <size in MB>]\t\t[-gts <size in MB>]\n\t[-ltl <length in minutes>]\t[-gtl <length in minutes>]" << std::endl;
    std::cout << "\t[-nsd | --new-scene-db ]\t[-usd | --update-scene-db]\n\t[-nad | -new-actor-db]\t\t[-uad | --update-actor-db]" << std::endl;
    std::cout << "\t[-sq | --sceneQuery] <field to search> <term to search for>\n\t[-aq | --actorQuery] <field to search> <term to search for>" << std::endl;   
}

void printBio (bio_t &bio)
{
	std::cout << std::endl;
	std::cout << "Name:\t\t" << bio.name << std::endl;
	std::cout << "Aliases:\t" << bio.aliases << std::endl;
	std::cout << "Birthday:\t" << bio.birthdate.wordString() << std::endl;
	std::cout << "Birthplace:\t" << bio.city << ", " << bio.nationality << std::endl;
    std::cout << "Nationality:\t" << bio.nationality << std::endl;
    std::cout << "Ethnicity:\t" << bio.ethnicity << std::endl; 
	std::cout << "Hair:\t\t" << bio.hairColor << std::endl;
	std::cout << "Eyes:\t\t" << bio.eyeColor << std::endl;
	std::cout << "Measurements:\t" << bio.measurements << std::endl;
    std::cout << "Height:\t\t" << bio.height.toString() << std::endl;
    std::cout << "Weight:\t\t" << bio.weight << " lbs.\n";
	std::cout << "Tattoos:\t" << bio.tattoos << std::endl;
	std::cout << "Piercings:\t" << bio.piercings << std::endl;
    std::cout << "Career Span: " << bio.start << " - " << bio.end << std::endl;
	std::cout << std::endl;
}

void printSceneList(std::vector<Scene> &s)
{
    std::cout << std::endl << "Scenes:" << std::endl;
    for (std::vector<Scene>::iterator S = begin(s); S != end(s); ++S)
    {
    	S->printStats();
	//cout << i << ":\t" << s.at(i).getTitle() << endl;
    }	
}

void saveSceneList(std::vector<Scene> &s, std::string logfile)
{
	std::ofstream outfile(logfile, std::ios::out | std::ios::trunc);
    for (int i = 0; i < s.size(); i++)
    {
    	outfile << s.at(i).toString();
    }	
    outfile << std::endl << "Total Number of Files Gathered:\t" << s.size() << std::endl; 
    outfile.close();
}

void printActressList(std::vector<Actor> &a)
{
    std::cout << "\nActresses:\n ";
    for (int i = 0; i < a.size(); i++){
		a.at(i).printScenes();
    }
    std::cout << "Total Number of Actresses in List:\t" << a.size() << std::endl;
}

void outputList(std::vector<Actor> &a, std::string name)
{
    if (name.empty())
        name = "Actors.txt";
    std::string fname = "./Data/" + name;
    std::fstream file(fname, std::ios::trunc);
    for (std::vector<Actor>::iterator A = begin(a); A != end(a); ++A)
        file << A->toString();
    std::cout << "Actress List output successfully." << std::endl;
}
void outputList(std::vector<Scene> &s, std::string name)
{
    if (name.empty())
        name = "Scenes.txt";
    std::string fname = "./Data/" + name;
    std::fstream file(fname, std::ios::trunc);
    for (std::vector<Scene>::iterator A = begin(s); A != end(s); ++A)
        file << A->toString();
    std::cout << "Actress List output successfully." << std::endl;   
}

void printDetails(std::vector<Scene> &s, std::vector<Actor> &a, std::string dir)
{
    printActressList(a);
    int totalFiles = s.size();
    int totalNames = a.size();
    double avg = (double)totalFiles/(double)totalNames;
    std::cout << totalNames << " Unique stars found in " << dir  << std::endl;
    std::cout << "Each star has appeared in an average of " << avg << " scenes " << std::endl << std::endl;
}
void toTextFile(std::vector<Filepath> &list, std::string name, bool newFile)
{
    std::fstream file;
    std::string content("");
    
    if (newFile)
    {
//      std::sort(begin(list), end(list), byFilename);
        file.open(name, std::ios::in|std::ios::out|std::ios::trunc);
        for (std::vector<Filepath>::iterator I = begin(list); I != end(list); ++I)
        {
            file << I->filename() << " in " << I->filepath() << std::endl;
        }
    }
    else
    {
    try{
        // if adding to a file, read in the current content so we can check before adding data, avoiding duplicates.
        file.open(name, std::ios::in);
        if (!file)
        {
            std::cout << "Error Opening File" << std::endl;
            return;
        }
        // Read in the file contents.
        while(file.good())
        {
            std::string temp("");
            getline(file, temp);
            content += temp;
        }
        file.close();
        // split the data read in by lines into a vector.
        std::vector<std::string> lines = tokenizeString(content, '\n');
        boost::regex fileRx("^(.+) in (.+)$");
        boost::smatch match;
        // add get the filepaths of the collected lines in the file and add them to the list 
        for (std::vector<std::string>::iterator l = begin(lines); l != end(lines); ++l)
        {
            if (boost::regex_search(*l, match, fileRx))
            {
                list.push_back(Filepath(match[1].str(), match[2].str()));
            }
        }
        file.open(name, std::ios::out|std::ios::trunc);// we're overwriting the old data, but we've already saved it so it will be re-written with the new data.

        // Sort the files by the directory they're in
        std::sort(list.begin(), list.end(), byFullpath);
        // Output the data to the text file.
        for (std::vector<Filepath>::iterator I = begin(list); I != end(list); ++I)
        {

            std::string line(I->filename());// + " in\t\t" + I->filepath() + "\n");
            file << line << std::endl;
        }
        file.flush();
        file.close();
    }catch(std::exception &e)
    {
        logError(e.what());
    }
    }
//    std::cout << "Done outputting data to " << name << std::endl; 
}
void toTextFile(std::vector<Scene> list, std::string name)
{
    std::string content("");
    std::fstream update_file(name,std::ios::out|std::ios::trunc);
    std::sort(begin(list), end(list), sortByActor);
    for (std::vector<Scene>::iterator I = begin(list); I != end(list); ++I)
    {
        update_file << I->getSize() << " MB\t" << I->getHeight() << "p\t" << I->getCompany() << ":\t" << I->filename() << std::endl;
    }

//    std::cout << "Done outputting data to " << name << std::endl; 
}
