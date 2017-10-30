#include <algorithm>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "Actor-old.h"
#include "Scene.h"
#include "database.h"
#include "lists.h"
#include "output.h"
#include "Filepath.h"   
#include "structs.h"
#include "stringTools.h"
#include "systemInterface.h"
#include "tools.h"
#include <dirent.h>
#include <sys/stat.h>
#include <boost/regex.hpp>
#include <omp.h>		// For Parallel Processing
#include <vector>
#define RUN_IN_PARALLEL
#define BUFFSIZE 4096 
#define NUM_THREADS 24

bool inList(QList<QStandardItem> &actorList, QString actorName)
{
    bool found = false;
    if (actorList.size() > 2){
        std::sort(actorList.begin(), actorList.end(), byName);
        if (list.size() == 1 && list.at(0).getName() == name)
            found = true;
        else
            found = !(findActor(actorList, actorName)->getName().isEmpty());
    }
    return found;
}


// Return a pointer to the Actor object containing the name specified by 'name'
Actor *findActor(QList<QStandardItem> &actorlist, QString actorname){
    return ActorSearch(actorlist, actorname, 0, list.size()-1);
}

// Recursively Search a list of Actor objects for a name 
Actor* ActorSearch(QList<QStandardItem>&list, QString name, int first, int last)
{
	static Actor a("");
	if (first <= last){
		int mid = (first + last)/2;
		if (name == (list.at(mid)).getName())
			return &(list.at(mid));
		else if ( name < (list.at(mid)).getName() )
			return ActorSearch(list, name, first, mid - 1);
		else	
			return ActorSearch(list, name, mid + 1, last);
	}
	return &a;
}

// Recursively search a list of scenes for the name of an actor
int SceneSearch(QList<QStandardItem>&scenelist, QString actorName, int actorNum, int first, int last)
{
     if (first <= last)
     {
	    int mid = (first + last)/2;
        if (actorName == (scenelist.at(mid)).getActorNumber(actorNum))
	    	return mid;
        else if ( actorName < (scenelist.at(mid)).getFirstActor() )
            return SceneSearch(scenelist, actorName, actorNum, first, mid - 1);
	    else	
            return SceneSearch(scenelist, actorName, actorNum, mid + 1, last);
    }
    return -1;
}
// Search list of scenes to see if a file exists in the list
bool SceneSearch(QList<Scene> &scenelist, Filepath f){
	bool found = false;
    for (QList<Scene>::iterator I = begin(scenelist); I != end(scenelist) && !found; ++I){
        found = (I->fullpath() == f.fullpath());
	}
	return found;
}
bool SceneSearch(QList<Scene> &list, Scene &scene){
	return SceneSearch(list, scene.getFile());
}

void getSceneList(QList<Filepath> &files, QList<Scene> &scene_library)//, runOptions_t opts, search_t queryStruct)
{
	int idx = 1;
	debug();
	
#ifdef RUN_IN_PARALLEL
	omp_set_dynamic(0); // disable automatic setting of count of threads
	#pragma omp parallel for num_threads(NUM_THREADS)
#endif
    for(size_t i = 0; i < files.size(); i++)
	{
			Scene s;
            if (!isSceneStored(files.at(i))){
				s = Scene(files.at(i));
			}else{
				s = loadScene(files.at(i));
			}
            #ifdef RUN_IN_PARALLEL
            #pragma omp critical
            {
                std::cout << idx++ << ": Adding:\t" << s.filename() << std::endl;
                scene_library.push_back(s);
            }
            #else
            std::cout << idx++ << ": Adding:\t" << s.filename() << std::endl;
            scene_library.push_back(s);
            #endif
	}
	std::cout << "****** FINISHED GATHERING SCENES ******" << std::endl << std::endl;
	//sort(scene_library.begin(), scene_library.end(), std::bind(sortByActorN, std::placeholders::_1, std::placeholders::_2, 0));
	sort(scene_library.begin(), scene_library.end(), sortByActor);
}


// Use a list of names & a list of scenes to create a std::vector of performers and their respective appearances
void getActressList(std::vector<Actor> &list, std::vector<Scene> &scenes)//, runOptions_t opts)
{
	std::vector<std::string> names(0);
    // Get List of names in string form
    #ifdef RUN_IN_PARALLEL
    omp_set_dynamic(0);
    #pragma omp parallel for num_threads(NUM_THREADS)
    #endif
    for (size_t i = 0; i < scenes.size(); i++){
        QStringList cast = (scenes.at(i)).getActors();
        for (int a = 0; a < cast.size(); ++a){
            QString m = cast.at(a);
            if (!(m.isEmpty() && m.contains("nknown"))){
                #ifdef RUN_IN_PARALLEL
                    #pragma omp critical
                    {
                        names.push_back(cast.at(a));
                    }
                #else
                    names.push_back(cast.at(a));
                #endif
            }
        }
//        for (size_t a = 0; a < cast.size(); a++)
//		{
//			if ((!(cast.at(a)).empty()) && (cast.at(a).find("nknown") == std::string::npos))
//    		{
//				#ifdef RUN_IN_PARALLEL
//					#pragma omp critical
//				    {
//				    	names.push_back(cast.at(a));
//				    }
//				#else
//				    names.push_back(cast.at(a));
//				#endif
//			}
//		}
    }
    // sort the names, then remove duplicates
    sort(names.begin(), names.end());
    names.erase(unique(names.begin(), names.end()), names.end());
    // Create an Actor object for each item in the list of names, and add them to a list of Actor Objects.
    for (auto const& N : names)
    {
        if (!inList(list, *N)){
            QList<Scene> locallist(0), temp(0);
            Actor a = loadActor(*N);
            getScenesWithActor(scenes, locallist, *N);
            a.addScenes(locallist);
            list.push_back(a);
            QString tabs = getTabString()
        }
    }
    for (std::vector<std::string>::iterator N = begin(names); N != end(names); ++N)
    {
    	if (!inList(list, *N)){
    		std::vector<Scene> localList(0), temp(0);//dbList(0), memList(0), finalList(0);	
    		Actor a = loadActor(*N);					// Either get new (empty) actor object, or one with data loaded from the database.
    		loadActorsScenes(localList, *N);			// Get All this actor's scenes from the Database
    		getScenesWithActor(scenes, localList, *N);	// Get All this actor's scenes from memory.
    		a.addScenes(localList);						// Add the Merged List to the Actor Object
    		list.push_back(a);							// Add the Actor Object to the List of Actors
    		std::string tabs = getTabString(*N+":", 2);
    		if (a.hasBio())
    			std::cout << *N << ":" + tabs + "added with " << localList.size() << " Scenes, + Bio" << std::endl;
    		else
    			std::cout << *N << ":" + tabs + "added with " << localList.size() << " Scenes" << std::endl;
    	}
    }
    addScenesToSecondaryActorProfiles(scenes, list);
    updateBios(list, scenes);
    retrieveFilmographies(list);
}



void getScenesWithActor(std::vector<Scene>&totalList, std::vector<Scene> &localList, std::string actorName)
{
    sort(totalList.begin(), totalList.end(), sortByActor);
    int location = SceneSearch(totalList, actorName, 0, 0, totalList.size()-1);
    if (location != -1 && location < (int)totalList.size())
    {
		Scene itemToAdd;
		try{itemToAdd = totalList.at(location);}
		catch(std::out_of_range &e){std::cerr << "Error attempting to access item " << location << " of 'list' at line " << __LINE__ << ":\t" << e.what() << std::endl;}
		//std::cout << "Adding " << list.at(location).filename() << std::endl;
		if (!SceneSearch(localList, itemToAdd) && !itemToAdd.empty()){
			addToList(localList, itemToAdd);
		}
		int before = location - 1;
		int after = location + 1;
	    try{
			if ((before > -1) && (totalList.at(before).getFirstActor() == actorName))
			{
				while((before > 0) && ((totalList.at(before)).getFirstActor() == actorName))
				{
					addToList(localList, totalList.at(before--));
				}
			}
	    }catch(std::out_of_range &e){ std::cerr << "Error adding scenes to actor's file: " << e.what() << std::endl;}
		try{
            if ((after < (int)totalList.size()) && ((totalList.at(after)).getFirstActor() == actorName))
		    {
                while((after < (int)totalList.size()) && ((totalList.at(after)).getFirstActor() == actorName))
			    {
			    	addToList(localList, totalList.at(after++));
			    }
		    }
		}catch(std::out_of_range &e){std::cerr << "Error adding scenes after the first one found to " << actorName << "'s profile: " << e.what() << std::endl;}
    }
}

// given a list of scenes SORTED by performer name, return a std::vector of all the scenes featuring
// the performer with the name given by 'actorName'
std::vector<Scene> getScenesWithActor(std::vector<Scene>&list, std::string actorName)
{
    std::vector<Scene> sceneList;
    int location = SceneSearch(list, actorName, 0, 0, list.size()-1);
    if (location != -1 && location < (int)list.size())
    {
		Scene itemToAdd;
		try{itemToAdd = list.at(location);}
		catch(std::out_of_range &e){std::cerr << "Error attempting to access item " << location << " of 'list' at line " << __LINE__ << ":\t" << e.what() << std::endl;}
		//std::cout << "Adding " << list.at(location).filename() << std::endl;
		if (!SceneSearch(sceneList, itemToAdd) && !itemToAdd.empty()){
			sceneList.push_back(itemToAdd);
		}
		int before = location - 1;
		int after = location + 1;
	    try{
			if ((before > -1) && (list.at(before).getFirstActor() == actorName))
			{
				while((before > 0) && ((list.at(before)).getFirstActor() == actorName))
				{
					sceneList.push_back(list.at(before--));
				}
			}
	    }catch(std::out_of_range &e){ std::cerr << "Error adding scenes to actor's file: " << e.what() << std::endl;}
		try{
            if ((after < (int)list.size()) && ((list.at(after)).getFirstActor() == actorName))
		    {
                while((after < (int)list.size()) && ((list.at(after)).getFirstActor() == actorName))
			    {
					sceneList.push_back(list.at(after++));
			    }
		    }
		}catch(std::out_of_range &e){std::cerr << "Error adding scenes after the first one found to " << actorName << "'s profile: " << e.what() << std::endl;}
    }
    return sceneList;
}


void retrieveFilmographies(std::vector<Actor> &a)
{
	std::cout << "Updating the Filmographies of the loaded actresses..." << std::endl;
	omp_set_dynamic(0);
	unsigned int updated = 0, loaded = 0, idx = 0;
	size_t list_size = a.size();
	#pragma omp parallel for num_threads(NUM_THREADS)
	for(size_t i = 0; i < a.size(); ++i)
	{
		if (!a.at(i).hasFilmography())
		{
			std::vector<Scene> films(0);
			// First try to load the filmography from the Database and assign it.
			if (loadActorsFilmography(films, a.at(i).getName()))
			{
				#pragma omp critical
				{
					++loaded;
					a.at(i).addFilmography(films);
				}
			}
			// If there's nothing to load in the table, resort to making a call to IAFD.
			else
			{
				a.at(i).fetchFilmography();
				size_t filmography_size = a.at(i).filmographySize();
				if (filmography_size > 0)
				{
					#pragma omp critical
					{
						std::cout << ++idx << "/" << list_size << " " << a.at(i).getName() << "'s IAFD Filmography Fetched.\t" << a.at(i).filmographySize() << " Scenes Listed." << std::endl; 
						++updated;
						storeFilmList(a.at(i).filmography());
					}
				}
			}

		}
	}
	std::cout << std::endl;
	std::cout << updated << "/" << list_size << "\tFilmographies updated from IAFD.\n";
	std::cout << loaded << "/" << list_size << "\tFilmographies loaded from the database\n";
	std::cout << (list_size - (loaded + updated)) << "/" << list_size << "\tFilmographies Unavailable\n\n" << std::endl;
}

void syncLists(std::vector<Scene> &sList, std::vector<Actor> &aList)
{
	std::sort(sList.begin(), sList.end(), sortByActor);
	for (size_t a = 0; a < aList.size(); a++)
	{
		Actor *actor = &aList[a];
		std::vector<Scene> sceneList = getScenesWithActor(sList, actor->getName());
        for (size_t s = 0; s < sceneList.size(); s++)
		{
			if (!actor->hasScene(sceneList.at(s)))
			{
				actor->addScene(sceneList.at(s));
			}	
		}
	}
	addScenesToSecondaryActorProfiles(sList, aList);
}

void addScenesToSecondaryActorProfiles(std::vector<Scene> &s, std::vector<Actor> &a)
{
	for (std::vector<Scene>::iterator it = s.begin(); it != s.end(); ++it)
	{
        size_t cast_size = it->getCastSize();
		// If the current scene has more than one actor
		if (cast_size > 1)
		{
			// go through all the actors (exculuding the first, which we assume has already been assigned this scene)
			for (size_t c_num = 1; c_num < cast_size; c_num++)
			{
				std::string name = it->getActorNumber(c_num);
				// there should be no reason for the actor not to be in the list, but check just in case.
				if (inList(a, name)){
					std::cout << "Adding Scene to " << name << "'s Profile" << std::endl;
					// WORKS!!! :D
					Actor *actor = findActor(a, name);
					if (!actor->hasScene(*it))
						actor->addScene(*(it));
				}
			}

		}
	}
}


void addToList(std::vector<Scene> &list, Scene &scene)
{
	if (!SceneSearch(list, scene)){
		list.push_back(scene);
	}
}

std::vector<Scene> mergeLists(std::vector<Scene> &listA, std::vector<Scene> &listB)
{
	std::vector<Scene> composite = listA;
	for (std::vector<Scene>::iterator I = begin(listB); I != end(listB); ++I)
	{
		if (!SceneSearch(composite, *I))
			composite.push_back(*I);
	}
	removeDuplicates(composite);
	return composite;
}


void removeDuplicates(std::vector<Scene> &list)
{
	// Start by sorting the list by filename.
	sort(list.begin(), list.end(), byFullpath);
	// Initialize Two indexes that we'll use to traverse the list.
	size_t curr = 0;
	size_t next = 1;
	// continue traversal while both indexes refer to valid list entries. 
	while (next < list.size())
	{
		try{
			std::string aName = list.at(curr).filename();
			std::string bName = list.at(next).filename();
			// if the elements @ curr & next have the same filename, erase the one at 'next'.
			// However, do not increment either index afterwards, as a new element will be at position 'next' following the deletion,
			// and we need to check if it is perhaps a triplicate entry.
			if (aName == bName){
				std::cout << "Removing Duplicate entry of " << bName << std::endl;
				list.erase(list.begin() + next);	
			}
			else
			{
				++next;
				++curr;
			}
		}catch(std::out_of_range &oor){
			std::cerr << oor.what() << std::endl;
		}
	}
}
void removeDuplicates(std::vector<Actor> &list)
{
	// Start by sorting the list by name.
	std::sort(list.begin(), list.end(), byName);
	// Initialize Two indexes that we'll use to traverse the list.
	size_t curr = 0;
	size_t next = 1;
	// continue traversal while both indexes refer to valid list entries. 
	while (next < list.size())
	{
		try{
			std::string aName = list.at(curr).getName();
			std::string bName = list.at(next).getName();
			// if the elements @ curr & next have the same filename, erase the one at 'next'.
			// However, do not increment either index afterwards, as a new element will be at position 'next' following the deletion,
			// and we need to check if it is perhaps a triplicate entry.
			if (aName == bName){
				std::cout << "Removing Duplicate entry of " << bName << std::endl;
				list.erase(list.begin() + next);	
			}
			else
			{
				++next;
				++curr;
			}
		}catch(std::out_of_range &oor){
			std::cerr << oor.what() << std::endl;
		}
	}	
}

