#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#include <boost/regex.hpp>
#include <omp.h>
#include "Actor-old.h"
#include "Scene.h"
#include "Filepath.h"
#include "output.h"
#include "systemInterface.h"
#include "lists.h"
#include "stringTools.h"
#include "database.h"

#define BUFFSIZE 4096 
#define NUM_THREADS 24


std::string systemCall(std::string c)
{
    std::string output = "";
    char buffer[BUFFSIZE];
    FILE *pipe = popen(c.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try{
        while (!feof(pipe)){
            if (fgets(buffer, BUFFSIZE, pipe)  != NULL)
                output += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return output;
}


bool fileWasRenamed(std::string path, std::string actor, std::string title, std::string &newFilename)
{
	bool success = false;
	std::string query("ls \"" + path + "\"\"" + actor+"\"*\"" + title + "\"*");
	std::string newName = systemCall(query);
	if (!newName.empty())
	{
		// We're not sophisticated enough to distinguish between 2 matching results,
		// so only consider the search successful if ONE result is returned (no more, no less).
		if(tokenizeString(newName, '\n').size() == 1)
		{
			newFilename = newName;
			success = true;
		}

	}
	return success;
}

// Recursive Function
std::vector<Filepath> getFiles(std::string dir)
{
	//cout << "Entering Directory " << dir << endl;
	//boost::regex filetype("^.+\\.(mpg|mp4|mov|mpeg|wmv|wma|flv)$");
	trimWhitespace(dir);
	std::vector<Filepath> files(0);
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(dir.c_str())) == NULL) 
	{
		std::cerr << "Couldn't open \'" << dir << "'" << std::endl;
		return files;
	}
	while((dirp = readdir(dp)) != NULL)
	{
		std::string name(dirp->d_name);
		Filepath entry(dir, dirp->d_name);
		//if (name != std::string(".") && name != std::string("..") && name.at(0) != '.' && name != std::string("#Images"))
		if (entry.filename().at(0) != '.')
		{
			if (isDir(entry.fullpath() + "/")){
				std::vector<Filepath> temp = getFiles(std::string(dir + entry.filename() + '/'));
				files.insert(files.end(), temp.begin(), temp.end());
			}
			else if (entry.video()){
					Filepath newFile(dir, name);	
					files.push_back(newFile);			
			}
		}
	}
	closedir(dp);
	return files;
}
// Recursive Function
std::vector<Filepath> getNewFiles(std::string dir)
{
	DIR *dp;
	struct dirent *dirp;
	std::vector<Filepath> newFiles(0);
	if ((dp = opendir(dir.c_str())) == NULL)	{ 	return newFiles;	}
	while((dirp = readdir(dp)) != NULL)
	{
		Filepath entry(dir, dirp->d_name);
		if (entry.filename().at(0) != '.')
		{
			if (isDir(entry.fullpath() + "/")){	// Recursive Case - Enter New Directory
				std::vector<Filepath> tempList = getNewFiles(entry.fullpath() + "/");
				newFiles.insert(newFiles.end(), tempList.begin(), tempList.end());
			}
			else if (entry.video() && !isSceneStored(entry)){	
					newFiles.push_back(entry);			
			}
		}
	}
	closedir(dp);
	return newFiles;
}
/*
void parseDirectory(std::vector<Scene> &s, std::string d)
{
	runOptions_t o;
	search_t g;
	parseDirectory(s, d, o, g);
}
*/
void parseDirectory(std::vector<Scene> &sceneList, std::string dir)//, runOptions_t opts, search_t search)
{
	std::cout << "Starting file search with " << dir << std::endl;
	std::string rootNode("");
	if (!dir.empty() && dir.at(0) == '~')	// if there's a tilde, replace it with the path to the user's home directory.
	{	
		rootNode = replaceString(dir, "~", HOME_DIR);
	}else{
		rootNode = dir;
	}
	// Get a list of Filenames of movies by recursively traversing the directory
	std::vector<Filepath> files = getFiles(rootNode);
	std::cout << files.size() << " files from " << dir << std::endl;
    getSceneList(files, sceneList);//, opts, search);
}


void	refreshDirectories(std::vector<std::string> directories)
{
	std::vector<Filepath> newFiles(0);
	for (std::vector<std::string>::iterator D = begin(directories); D != end(directories); ++D)
	{
		std::string dir = *D;
		if (dir.at(dir.size() - 1) != '/')	{	dir.push_back('/');	}	// add a slash to the end if there isn't one, to enable recursive behaviour.
		std::vector<Filepath> temp = getNewFiles(dir);				// get files from this directory
		newFiles.insert(newFiles.end(), temp.begin(), temp.begin());	// append them to the master list.
	}
	addNewItems(newFiles);
}


void addNewItems(std::vector<Filepath> &fileList)
{
	int idx = 0;
	std::vector<Scene> sceneList(0);	// temporary (local) scene list
	std::vector<Actor> actorList(0);	// temporary (local) actor list
	omp_set_dynamic(0); // disable automatic setting of count of threads
	#pragma omp parallel for num_threads(NUM_THREADS)
	for(int i = 0; i < fileList.size(); i++)
	{

		size_t lastcharpos = fileList.at(i).fullpath().size() - 1;
		char lastchar = fileList.at(i).fullpath().at(lastcharpos);
		if ((lastchar != '/') && (!SceneSearch(sceneList, fileList.at(i))))
		{
			Scene newScene(fileList.at(i));
			#pragma omp critical 
			{
			    std::cout << ++idx << ": Adding:\t" << newScene.filename() << std::endl;
			    sceneList.push_back(newScene);	
			}	
		}
	}
	std::cout << "****** FINISHED GATHERING SCENES ******" << std::endl << std::endl;
	sort(sceneList.begin(), sceneList.end(), std::bind(sortByActorN, std::placeholders::_1, std::placeholders::_2, 0));
	getActressList(actorList, sceneList);		// Update List of Actors, Retrieve any Relevant records, and update any empty bios.
	storeSceneList(sceneList);					// Store any new scenes in the list to the Database
	storeActorList(actorList);					// Store any new Actors to the database
}

bool criteriaMatch(Scene &s, runOptions_t opts, search_t queryStruct)
{
	bool addScene = true;
	if (s.notNull() && !s.filetype().empty())
	{
		if (queryStruct.ratedOnly || queryStruct.size || queryStruct.length)
		{
			addScene = false;	// Initial setting. must Pass criteria to re-gain the true status.
			if (queryStruct.size)			// If testing against filesize
			{
				if (queryStruct.lt)			// if smaller than n MB
				{
					if (queryStruct.number > s.getSize())
						addScene = true;
				}
				else if (queryStruct.gt)	// if greater than N megabytes
				{
					if (queryStruct.number < s.getSize())
						addScene = true;
				}
			}
			else if (queryStruct.length)	// If Testing against Video Length
			{
				if (queryStruct.lt)
				{
					if (queryStruct.number > s.getLength())	// if shorter than n Minutes
						addScene = true;
				}
				else if (queryStruct.gt)
				{
					if (queryStruct.number < s.getLength())	// if longer than n minutes
						addScene = true;
				}
			}

			if (queryStruct.ratedOnly)
			{
				if (!s.rated())	// Set any previously true status to false if this condition is required but not met
					addScene = false;
				else if (!queryStruct.size && !queryStruct.length && (s.rated()))
					addScene = true;
			}
		}
	}
	else
	{
		addScene = false;
	}
	return addScene;
}
