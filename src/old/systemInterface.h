#ifndef __SYSTEM_INTERFACE_H__
#define __SYSTEM_INTERFACE_H__
#include <vector>
#include "Filepath.h"
#include "structs.h"

std::string 			systemCall 	(std::string);
std::vector<Filepath>	getFiles	(std::string dir);
std::vector<Filepath> 	getNewFiles (std::string dir);
void					addNewItems (std::vector<Filepath> &);
bool 					criteriaMatch	(class Scene &, runOptions_t, search_t);
void					refreshDirs 	(std::vector<std::string>);
void					parseDirectory	(std::vector<class Scene> &, std::string dir);
//void parseDirectory(std::vector<Scene> &sceneList, std::string dir);//, runOptions_t opts, search_t search);
bool fileWasRenamed(std::string path, std::string actor, std::string title, std::string &newFilename);

#endif
