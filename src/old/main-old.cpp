
#include <algorithm>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include "Actor-old.h"
#include "Scene.h"
#include "interface.h"
#include "lists.h"
#include "database.h"
#include "output.h"
#include "Filepath.h"   
#include "structs.h"
#include "tests.h"
#include "systemInterface.h"

using namespace std;
//using namespace chrono;

void parseArgs(char *argv[], std::vector<std::string> &directories, runOptions_t &opt, search_t &search){
    int idx = 0;
    bool error = false;
    std::string errStr = "";
    std::string configStr = "";
    while (argv[++idx] && !error)
    {
		string currArg (argv[idx]);
		// Print Usage Info
		if (currArg == "-h" || currArg == "--help"){
			printUsage();
			exit(0);
		}
		else if (currArg == "-i" || currArg == "--interactive")
		{
			configStr += "Running in Interactive Mode.";
			opt.interactive = true;
		}
		// Don't Get anything from Sub-Directories
		else if (currArg == "-n" || currArg == "--non-recursive"){
			opt.recursive = false;
		}
		// This is a directory
		else if (currArg.find("/") != std::string::npos)
		{
		    configStr += "Adding " + currArg + " to search path.";
		    
		    if (currArg.at(currArg.size() - 1) != '/')
		    	currArg.push_back('/');

		    directories.push_back(currArg);
		}
		// Supress Output
		else if (currArg == "-s" || currArg == "--silent")
		    opt.silent = true;
		// Run Test Suite rather than Standard Operations
		else if (currArg == "-t" || currArg == "--test"){
		    opt.test = true;
		   // configStr += "Running Test Suite.\n";
		}
		else if (currArg == "-o" || currArg == "--output")
		{
			if (argv[idx+1])
			{
				opt.outputLog = std::string(argv[++idx]);
				opt.outputToFile = true;
				configStr += "Saving Output to " + opt.outputLog + "\n";
			}
			else {
				error = true;
				errStr = "Did not provide file name to save output to.";
			}
		}
		else if (currArg == "-ud" || currArg == "--use-database" || currArg == "--use-databases")
		{
			opt.useDatabases = true;
		}
		// Print Out Data on Scenes Gathered
		else if (currArg == "-pa" || currArg == "--print-actors" || currArg == "--print-actresses")
		{
			opt.printActresses = true;
		}
		else if (currArg == "-ps" || currArg == "--print-scenes")
		{
			opt.printScenes = true;
		}
		// Create new Scene Database
		else if (currArg == "-nsd" || currArg == "--new-scene-db"){
		    opt.newSceneDb = true;
			opt.useDatabases = true;
		    configStr += "Creating new Scene Database with files in provided directories.\n";
		}
		// Update the Scene Database
		else if (currArg == "-usd" || currArg == "--update-scene-db"){
			opt.updateSceneDb = true;
			opt.useDatabases = true;
			configStr += "Updating Existing Scene Database with files gathered from provided directories\n";
		}
		// Create a new Actor Database
		else if (currArg == "-nad" || currArg == "--new-actor-db"){
		    opt.newActorDb = true;
			opt.useDatabases = true;
			configStr += "Creating New Actor Database with files gathered from provided directories\n";
		}
		// Update the existing Actor Database
		else if (currArg == "-uad" || currArg == "--update-actor-db"){
			opt.updateActorDb = true;
			opt.useDatabases = true;
			configStr += "Updating Existing Actor Database with files gathered from  provided directories\n";
		}
		else if (currArg == "-ga" || currArg == "--get-ages"){
			opt.getAges = true;
			configStr += "Adding age of actors to videos wherever possible. This may require some extra processing time.\n";
		}
		else if (currArg == "-fa" || currArg == "--find-actor")
		{
			if (argv[idx+1]){
				search.set = true;
				opt.findActor = true;
				std::string temp(argv[++idx]);
				configStr += "Gathering Data About " + temp + " and finding all relevant videos in the directories given.\n";
				std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
				search.query = temp;
			}else{
				error = true;
				errStr = "No Name Specified";	
			}
		}
		// Provide a Search Term to Use
		else if (currArg == "-f" || currArg == "--find"){
			if (argv[idx+1]){
				search.set = true;
				std::string temp(argv[++idx]);
				std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
				search.query = temp;
				configStr += "Searching " + std::string(argv[1]) + " for \"" + search.query + "\"\n";
			}else{
				error = true;
				errStr = "No Search Term Specified";
			}
		}
		else if (currArg == "-hr" || currArg == "-rated" || currArg == "--has-rating" || currArg == "--rated")
		{
			search.set = true;
			search.ratedOnly = true;
			configStr += "Only Looking for rated Scenes\n";
		}
		else if (currArg == "-gts"){
			if (argv[idx+1]){ 			// TODO: Add check that the next argument is a number
				search.set = true;
				search.gt = true;
				search.size = true;
				search.number = (double)atoi(argv[++idx]);
				configStr += "Looking for files greater than " + to_string(search.number) + " MB in provided directories\n";
			}else{	error = true;	}
		}
		else if (currArg == "-lts"){
			if (argv[idx+1]){ 			// TODO: Add check that the next argument is a number
				search.set = true;
				search.lt = true;
				search.size = true;
				search.number = (double)atoi(argv[++idx]);
				configStr += "Looking for files smaller than " + to_string(search.number) + " MB in provided directories\n";
			}else{	error = true;	}	
		}
		else if (currArg == "-gtl"){
			if (argv[idx+1]){ 			// TODO: Add check that the next argument is a number
				search.set = true;
				search.gt = true;
				search.length = true;
				search.number = (double)atoi(argv[++idx]);
				configStr += "Looking for files longer than " + to_string(search.number) + " minutes in provided directories\n";
			}else{	error = true;	}
		}
		else if (currArg == "-ltl"){
			if (argv[idx+1]){ 			// TODO: Add check that the next argument is a number
				search.set = true;
				search.lt = true;
				search.length = true;
				search.number = (double)atoi(argv[++idx]);
				configStr += "Looking for files less than " + to_string(search.number) + " minutes in provided directories\n";
			}else{	error = true;	}
		}
		// Create a SceneQuery Object - Used for Advanced Searches.
		else if (currArg == "-sq" || currArg == "--sceneQuery") {
			if(argv[idx+1] && argv[idx+2]){
				opt.queryScenes = true;
				opt.sceneQuery.field = std::string(argv[++idx]);
				opt.sceneQuery.term  = std::string(argv[++idx]);
			}else{
				errStr = "Too few arguments provided to perform a Scene Query";
				error = true;
			}
		}
		// Create an ActorQuery Object - Used for Advanced Searches.
		else if ((currArg == "-aq" || currArg == "--actorQuery")){	
			if(argv[idx+1] && argv[idx+2]){
				opt.queryActors = true;
				opt.actorQuery.field = argv[++idx];
				opt.actorQuery.term = argv[++idx];
			}else{
				errStr = "Too few arguments provided to perform an Actor Query.";
				error = true;
			}
		}
    }
   // Either print error message and exit, or print configuration/operation details and return to main.
    if (error){
    	std::cerr << errStr << std::endl << std::endl; 
    	printUsage();
    	exit(-1);
    }else{
    	std::cout << configStr << std::endl;
    }

}

int main(int argc, char *argv[])
{
	runOptions_t opt;		// Struct of operationl options that are set by flags passed via the command line.
	search_t queryStruct;	// Struct for holding info for basic query operations/data.
	std::vector<std::string> directories(0);
	std::vector<Scene> scenes(0);
	std::vector<Actor> actresses(0);
	startSqlServer();
	parseArgs(argv, directories, opt, queryStruct);
//	double t_start = (double)clock() / (double)CLOCKS_PER_SEC;
	initDate();
	if (argc == 1 || opt.interactive)
	{
	    interactiveMode(scenes, actresses);
	}
	else if (std::string(argv[1]) == "-t")
	{
		if (argv[2] != NULL)
		{
			std::string name(argv[2]);
			testIAFDquery(name);
		}
		else
		{
			runTests();
		}
	}
	else if (directories.size() > 0)
	{
	    //-------------------------------------------------------------
	    //					DATA GATHERING
	    //-------------------------------------------------------------	    
	    // Parse the List of Filenames into organized Data Structures
	    for (size_t i = 0; i < directories.size(); i++)
	    {
			std::vector<Scene> temp(0);
			parseDirectory(temp, directories.at(i));
			//parseDirectory(temp, directories.at(i), opt, queryStruct);
			scenes.insert(scenes.end(), temp.begin(), temp.end());
			getActressList(actresses, scenes);
	    }
	    // If we need a list of actresses for the chosen task, gather that list.
	   // if (opt.newActorDb || opt.updateActorDb || opt.printActresses || opt.getAges)	
	    //{	getActressList(actresses, scenes, opt);	}
	    //-------------------------------------------------------------
	    //					DATA OUTPUT
	    //-------------------------------------------------------------	    
	    if (opt.printScenes)	{	printSceneList(scenes);			}
	    if (opt.printActresses)	{	printActressList(actresses); 		}
	    //if (opt.outputToFile)	{	saveSceneList(scenes, opt.outputLog); 	}
	    //-------------------------------------------------------------
	    //					DATABASE ROUTINES
	    //-------------------------------------------------------------
/*
	    if (opt.newSceneDb)
	    {
			std::cout << "Creating Scene Database..." << std::endl;
			createSceneCSV(scenes, "./data/scenes.csv");
			std::cout << "\nDONE!\n\n";
	    }
	    if (opt.newActorDb)
	    {
			std::cout << "Creating Actor Database..." << std::endl;
			createActorCSV(actresses, "./data/actors.csv");
			std::cout << "\nDONE!\n\n";
	    }
	    if (opt.updateActorDb){
	    	// TODO: Add update code
	    }
	    if (opt.updateSceneDb){
	    	purgeSceneTable();	// Remove entries that have been deleted, moved or renamed.
	    	// TODO: Add update code for adding new files and ignoring ones with entries already in the database
	    }
*/
	    interactiveMode(scenes, actresses);
	}
	return 0;
}
