#include <algorithm>
#include <boost/regex.hpp>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <sstream>
#include <string>
#include <vector>
#include <readline/readline.h>
#include <readline/history.h>

#include "Actor-old.h"
#include "Scene.h"
#include "collectors.h"
#include "curlRequests.h"
#include "database.h"
#include "imageTools.h"
#include "interface.h"
#include "lists.h"
#include "output.h"
#include "stringTools.h"
#include "Filepath.h"   
#include "structs.h"
#include "systemInterface.h"
#include "tests.h"
#include "tools.h"

#define NUM_THREADS 24
static const std::string HOMEDIR("/Users/derby/");
static const std::string WHITDIR("/Volumes/White_8TB/");
static const std::string DARKDIR("/Volumes/White_8TB/");
static const std::string LITEDIR("/Volumes/4TB_Black/");
#define VRDIR	"/Volumes/500GB_Blue/"
#define DEFAULT_QUALITY 480
#define DEFAULT_SIZE 300
using namespace std;
const std::string updateActorsTable("update actors");
const std::string updateScenesTable("update scenes");
const std::string clearSceneTable("clear scenes");
const std::string clearActorsTable("clear actors");
const boost::regex readDirRx("(read|parse|import) (.+)");

std::string getAbsolutePath(std::string oldPath)
{
	std::string s = oldPath;
	trimWhitespace(s);
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	std::string absPath("");
	if (!s.empty()){
		if (s.at(0) == '~'){
			absPath = HOMEDIR;
			if (s.size() > 1)
				absPath.append(s, 2, std::string::npos);	// append the rest of the string 
		}
		else if (s == "brunette")
			absPath = DARKDIR + "Brunette8TB/";
		else if (s == "raven")
			absPath = DARKDIR + "Raven8TB/";
		else if (s == "alt")
			absPath = DARKDIR + "AltGirls8TB/";
		else if (s == "blonde")			
			absPath = LITEDIR + "Blonde4TB/";
		else if (s == "redhead")
			absPath = LITEDIR + "Redhead4TB/";
		else if (s == "amateur")		
			absPath = LITEDIR + "Amateur/";
		else if (s == "dark euro")
			absPath = DARKDIR + "Euro_Dark/";
		else if (s == "latina")
			absPath = DARKDIR + "Latina/";
		else if (s == "collections4")	
			absPath = LITEDIR + "#Collections4TB/";
		else if (s == "collections2")
			absPath = LITEDIR + "#Collections2TB/";
		else if (s == "collections8")
			absPath = WHITDIR + "#Collections8TB/";
		else if (s == "downloads")
			absPath = HOMEDIR + "Downloads/";
		else 
			absPath = oldPath;
	}
	return absPath;
}


void printHelp(void)
{
	std::string h("");
	h+= "\nPATH SHORTHANDS:\n\n";
	h+= "'~'-------------->" + getAbsolutePath("~") + "\n";
	h+= "'blonde'--------->" + getAbsolutePath("blonde") + "\n";
	h+= "'brunette'------->" + getAbsolutePath("brunette") + "\n";
	h+= "'raven'---------->" + getAbsolutePath("raven") + "\n";
	h+= "'latina'--------->" + getAbsolutePath("latina") + "\n";
	h+= "'redhead'-------->" + getAbsolutePath("redhead") + "\n";
	h+= "'alt'------------>" + getAbsolutePath("alt") + "\n";
	h+= "'amateur'-------->" + getAbsolutePath("amateur") + "\n";
	h+= "'collections4'--->" + getAbsolutePath("collections4") + "\n";
	h+= "'collections8'--->" + getAbsolutePath("collections8") + "\n";
	h+="\n\nAvailable Commands:\n";
	h+="parse:\t<directory path>\tRead in a list of files from a directory and create Scene Objects for them.\n";
	h+="print\t<scenes/actors>:\tPrint the list of * in formatted form to the command window.\n";
	h+="store\t<scenes/actors>:\tSave * list currently in memory to the database.\n";
	h+="load\t<scenes/actors>:\tRead entries from the * Database into the list of * Objects in memory\n";
	h+="clear\t<scenes/actors>:\tClear the * list currently in memory without affecting the database.\n";
	h+="make\t<scene/actor> table:\tCreate * table in the database, if one does not already exist.\n";
	h+="clear\tlists:\t\t\tClear all Scene & Actor data currently held in memory.\n";
	h+="clear\t<scene/actor> table:\tClear all entries out of * table.\n";
	h+="purge\t<scene/actor> table:\tRemove outdated/obsolete entries from the * table.\n";
	h+="free\t<scenes/actors>:\tDelete the Specified Table from the database.\n";
	h+="get actors:\t\t\tUse the list of Scene Objects in memory to compile the list of Actors that Appear in them.\n";
	h+="update\tbios/get bios:\t\tGet biographical data about all actors in the list from the web.\n";
	h+="write <actor/scene> csv:\tWrite all * in memory to *.csv, overwriting any previous content it might have held.\n";
	h+="add to <actor/scene> csv:\tWrite all * in memory to *.csv, adding to any previous content it might have held.\n";
	h+="new update list:\t\tOutput a list of files that are worth looking into getting higher quality versions of\n";
	h+="add to update list:\t\tadd any newly read in scenes that fit updatable criteria to the list\t";
	h+="output\t<scenes/actors>: <filename.txt>\n\t\t\t\tSave Formatted output of list currently in memory to text file.\n\t\t\t\t(if no name provided, file will be named after the list)\n";
	//h+="\ninit\n\tStart or Restart the PostGreSQL Server\n";
	h+="system: <command>:\t\tRun a unix system command (Unsafe, Testing Only)\n";
	h+="sql <query>\t\t\tRun a custom SQL query directly on the database (Unsafe, Testing Only)\n";
	h+="run tests\t\t\trun the test suite on the code bank, found in test.cpp\n";
	h+="test renaming:\t\tTest the renaming function on all the scenes currently in memory. No actual renaming will occur.\n";
	h+="commit renaming:\t\tPerform automated renaming of all files in memory. Files will indeed be renamed.\n";
	h+="company list for \"Actor Name\":\tQuery the IAFD profile for an actor/actress and retrieve a list of companies they've shot with,\n\t\t\t\tand the number of films available from each.\n";
	
	std::cout << h << std::endl;
}

void printPrompt(void){
	std::cout << PROMPT;
}

int getNumber(std::string prompt, bool requireAnswer = false)
{
	bool inputValid = false;
	int input_num = 0;
	std::string input("");	
	while (!inputValid)
	{
		cout << prompt << ": ";
		getline(cin, input);
		stringstream inStream(input);
		
		if (inStream >> input_num){
			inputValid = true;
			cout << endl << endl;
		}
		else if (!requireAnswer && input.empty())
		{
			input_num = -1;
		}
		else
			cout << endl << "\"" << input << "\" is not a valid number.\n" << endl;
	}
	return input_num;
}

bool getYesNo(std::string prompt, bool defaultAnswer)
{
	bool inputValid = false;
	bool choice;
	std::string input("");
	while(!inputValid)
	{
		inputValid = true;
		cout << prompt << " [y(es)/n(o)]: ";
		getline(cin, input);
		std::transform(input.begin(), input.end(), input.begin(), ::tolower);
		if (input.empty())
			choice = defaultAnswer;
		else if (input == "y" || input == "yes")
			choice = true;
		else if (input == "n" || input == "no")
			choice = false;
		else 
			inputValid = false;
	}
	return choice;
}

bool doubleCheck(void)
{
	std::string input;
	std::cout << "Are you sure? (yes/no): ";
	cin >> input;
	trimWhitespace(input);
	//std::transform(input.begin(), input.end(), input.begin(), ::tolower);
	if (input == "yes")
	{
		return true;
	}
	else 
	{
		std::cout << "\nPussy.\n" << std::endl;
		return false;
	}
}

void purge(std::string tablename)
{
	if (tablename.empty() || contains(tablename, "scene")){
		purgeSceneTable();
	}
	else if(contains(tablename, "actor")){
		purgeActorTable();
	}
	else{
		std::cout << "Unrecognized Table name" << std::endl;
	}
}

void clearList(std::string listName, std::vector<Actor> &a, std::vector<Scene> &s, std::vector<Scene> &o)
{
	if (listName.empty()||contains(listName, "scene"))
		s.resize(0);
	else if (contains(listName, "actor"))
		a.resize(0);
	else if (contains(listName, "op"))
		o.resize(0);
	else
		cout << "Unrecognized List name" << endl;
}

void clearTable(std::string table)
{
	if (contains(table, "actor")){
		std::cout << "Deleting all entries from Actor Table" << std::endl;
	    if (doubleCheck())	writeSQL_noResponse("DELETE FROM actors;"); 
	}
	else if (contains(table, "scene"))
	{
		std::cout << "Deleting all entries from Scene Table" << std::endl;
	    if (doubleCheck())	writeSQL_noResponse("DELETE FROM scenes;"); 
	}
	else
	{
		cout << "Unrecognized Table Name" << endl;
	}

}

void newUpdateList(void)
{
	bool ratedOnly;
	int size, quality;

	std::vector<Scene> scenes(0);
	quality = getNumber("Using cutoff quality");
	size = getNumber("Using cutoff Size");
	ratedOnly = getYesNo("Collect only Scenes with Ratings?", false);

	if (quality == -1)
		quality = DEFAULT_QUALITY;
	if (size == -1)
		size = DEFAULT_SIZE;
	
	loadScenes(scenes);
	toUpdate(scenes, true, quality, size, ratedOnly);
}

void interpereter2 (std::string input, std::vector<Actor> &actors, std::vector<Scene> &scenes, std::vector<Scene> &opBuffer)
{
	bool s_populated = false;
	bool a_populated = false;
	if (actors.size() > 0)	a_populated = true;
	if (scenes.size() > 0)	s_populated = true;

	vector<string> argv = tokenizeString(input, ' ');
	size_t argc = argv.size();
	std::string command = argv[0];
	if (argc == 1)
	{
		argv.push_back("");
		argv.push_back("");
	}

	// Clean Tables
	if (command == "purge"){

		purge(argv[1]);
	}
	// Clear Lists/Tables
	else if (command == "clear")
	{
		if (contains(argv[2], "table"))
			clearTable(argv[1]);
		else
			clearList(argv[1], actors, scenes, opBuffer);
	}
	else if (command == "parse")
	{
		if (!argv[1].empty()){
			parseDirectory(scenes, removeBefore(input, "parse"));
			getActressList(actors, scenes);
		}
	}
	else if (command == "readin")
	{
		if (!argv[1].empty()){
			parseDirectory(scenes, removeBefore(input, "parse"));
			getActressList(actors, scenes);
			storeActorList(actors);
			storeSceneList(scenes);
		}
	}

	else if (command == "download")
	{
		downloadHeadshots(actors);
	}
	else if (argv[1] == "reset")
	{
		if (command == "soft")
			softResetBios();
		else if (command == "hard")
			hardResetBios();
	}
	else if (command == "generate")
	{
		generateThumbnails(scenes);
	}
	else if (command == "load")
	{
		if (contains(argv[1], "scene"))
			loadScenes(scenes);
		else if (contains(argv[1], "actor"))
			loadActors(actors);
	}
	else if (command == "store")
	{
		if (contains (argv[1], "scene"))
			storeSceneList(scenes);
		else if (contains (argv[1], "actor"))
			storeActorList(actors);
		
	}
	else if (command == "update" && argv[1] == "weights")
	{
		updateWeights();
	}
	else if (command == "print")
	{
		if (contains (argv[1], "scene"))
			printSceneList(scenes);
		else if (contains (argv[1], "actor"))
			printActressList(actors);	
	}
	else if (command == "output")
	{
		if (contains(argv[1], "actor"))
			outputList(actors, argv[2]);
		else if (contains(argv[1], "scene"))
			outputList(scenes, argv[2]);
		else if (contains(argv[1], "update"))
			newUpdateList();

	}
	else if (command == "new")
	{
		if (contains(argv[1], "scene"))
			createTable(SCENE_TABLE);
		else if (contains(argv[1], "actor"))
			createTable(ACTOR_TABLE);
		else if (contains(argv[1], "film"))
			createTable(FILMOGRAPHY_TABLE);
		else if (contains(argv[1], "thumbnail"))
			createTable(THUMBNAIL_TABLE);
		else if (contains(argv[1], "headshot"))
			createTable(HEADSHOT_TABLE);
		else if (contains(argv[1], "update"))
			newUpdateList();
	}
	else if (command == "test")
	{
		if (argc > 2){
			std::string name("");
			for (size_t i = 2; i < argc; ++i)
			{	
				if (i > 2)
					name += " ";
				name += argv[i];
			}	
			std::cout << "Running Test \'" << argv[1] << "\' for \'" << name << "\'" << std::endl;
			if (contains(argv[1], "headshot"))
				testImageDownload(name);
			else if (contains(argv[1], "iafd"))
				testIAFDquery(name);
			else if (contains(argv[1], "height"))
				testHeightAndWeight(name);
		}
	}

	

}

void interpereter(std::string input, std::vector<Actor> &actors, std::vector<Scene> &scenes, std::vector<Scene> &opBuffer)
{
	std::vector<std::string> argv = tokenizeString(input, ':');
	trimWhitespace(input);
	int argc = (int)argv.size();
	bool s_populated = false;
	bool a_populated = false;
	if (actors.size() > 0)
		a_populated = true;
	if (scenes.size() > 0)
		s_populated = true;
	runOptions_t opt;
	search_t search;
	std::string command = argv.at(0);
	if (contains(command, "parse"))
	{
		removeStr(command, "parse", true);
		// Read files from a directory and create scene objects.
		size_t oldsize = scenes.size();
		opt.recursive = true;
        parseDirectory(scenes, getAbsolutePath(command));//, opt, search);
		if (oldsize < scenes.size())
		{
		    getActressList(actors, scenes);
		}
	}
	else if (command == "purge")					{	purgeSceneTable();	}
	else if (command == "purge actor table")		{	purgeActorTable();	}
	else if (command == "new scene table" || command == "make scene table"){createTable(SCENE_TABLE);}
	else if (command == "new actor table" || command == "make actor table"){createTable(ACTOR_TABLE);}
	else if (command == "new film table")	{createTable(FILMOGRAPHY_TABLE);}
	else if (command == "get bios")		{updateBios(actors, scenes);	}
	else if (command == "clear scenes"){scenes.resize(0);}
	else if (command == "clear actors"){actors.resize(0);}
	else if (command == "clear lists"){
			scenes.resize(0);
			actors.resize(0);
	}
		else if (command == "load scenes")
	{
		// Read scene list in from database
		loadScenes(scenes);
	}
	else if (command == "load actors")
	{
		// Read actor list in from database
		loadActors(actors);
	}
	else if (command == "store scenes" || command == "save scenes")
	{
	    std::cout << "Adding Data collected on scenes to the database" << std::endl;
	    storeSceneList(scenes);
	}
	else if (command == "store actors" || command == "save actors")
	{
	    std::cout << "Adding Data collected on Actors to the database" << std::endl;
	    storeActorList(actors);
	}
	else if (command == "print actors" || command == "print actresses")
	{
		if (a_populated)
			printActressList(actors);
		else
			std::cout << "Load some, first!" << std::endl;
	}
	else if (command == "print scenes")
	{
		if (s_populated)
			printSceneList(scenes);
		else
			std::cout << "Load some, first!" << std::endl;
	}
	

	else if (command == "test update"){
	    for (std::vector<Actor>::iterator I = actors.begin(); I != end(actors); ++I)
			std::cout << I->sqlUpdate() << std::endl << I->sqlInsert() << std::endl << std::endl;
	}else if (command == "test scene update"){
	    for (std::vector<Scene>::iterator I = scenes.begin(); I != end(scenes); ++I)
			std::cout << I->sqlUpdate() << std::endl << I->sqlInsert() << "\n\n";
	}
	else if (command == "new update list")
	{
		loadScenes(scenes);
	    if (argc > 2 && isNumber(argv[1]) && isNumber(argv[2]))
	    	toUpdate(scenes, true, atoi(argv[1].c_str()), atoi(argv[2].c_str()));
	    else if (argc > 1 && argv[1] == "rated")
	    	toUpdate(scenes, true, 720, 700, true);
	    else if (argc > 1 && isNumber(argv[1]))
	    	toUpdate(scenes, true, atoi(argv[1].c_str()));
	    else
	    	toUpdate(scenes, true);
	}
	else if (command == "number scenes"){}
	else if (command == "number actors"){}
	else if (command == "added before"){
		if (!argv[1].empty())
			listScenesAddedBefore(textDateToStruct(argv[1]), "Scenes Added Before " + argv[1] + ".txt");
	}else if (command == "added after"){
		if (!argv[1].empty())
			listScenesAddedAfter(textDateToStruct(argv[1]), "Scenes Added After " + argv[1] + ".txt");
	}else if (command == "added on"){
		if (!argv[1].empty())
			listScenesAddedOn(textDateToStruct(argv[1]), "Scenes Added On " + argv[1] + ".txt");
	}else if (command == "released before"){
		if (!argv[1].empty())
			listScenesReleasedBefore(textDateToStruct(argv[1]), "Scenes Released Before " + argv[1] + ".txt");
	}else if (command == "released after"){
		if (!argv[1].empty())
			listScenesReleasedAfter(textDateToStruct(argv[1]), "Scenes Released After " + argv[1] + ".txt");
	}else if (command == "released on"){
		if (!argv[1].empty())
			listScenesReleasedOn(textDateToStruct(argv[1]), "Scenes Released On " + argv[1] + ".txt");
	}else if (command == "get series")
	{
		for (std::vector<Scene>::iterator s = scenes.begin(); s != scenes.end(); ++s)
		{
			std::string series = s->getSeries();
			if (!series.empty())
				std::cout << s->getSeries() << std::endl;
		}
	}
	else if (command == "free actors"){
		std::cout << "Deleting Actor table" << std::endl;
		if (doubleCheck())
			writeSQL_noResponse("drop table actors;");
	}
	else if (command == "free scenes"){
		std::cout << "Deleting Scene table" << std::endl;
		if (doubleCheck())
			writeSQL_noResponse("drop table scenes;");
	}
	else if (command == "write actor csv"){
	    if (!a_populated)
		std::cout << "Empty List!" << std::endl;
	    else{
		try{
		    std::ofstream f("actors.csv", std::ios::trunc);
		    for (std::vector<Actor>::iterator a = actors.begin(); a != actors.end(); ++a)
		    {
				f << a->sqlInsert() << std::endl;
		    }
		    f.close();
		    std::cout << "Done! " << actors.size() << " entries written to actors.csv" << std::endl;
		}catch(std::exception &e){std::cerr<<__FILE__<<" @ " <<__LINE__<< ": " << e.what() << std::endl;}
	    }
	}
	else if (command == "write scene csv")
	{
	    if (!s_populated)
		std::cout << "Empty List!\n";
	    else{
		try{
		    std::ofstream f("scenes.csv", std::ios::trunc);
		    for (std::vector<Scene>::iterator s = scenes.begin(); s != scenes.end(); ++s)
		    {
				f << s->sqlInsert() << std::endl;
		    }
		    f.close();
		    std::cout << "Done! " << scenes.size() << " entries written to scenes.csv" << std::endl;
		}catch(std::exception &e){std::cerr<<__FILE__<<" @ " <<__LINE__<< ": " << e.what() << std::endl;}
	    }
	}
	else if (command == "add to actor csv")
	{
	    if (!a_populated)
		std::cout << "Empty List!" << std::endl;
	    else{
		std::ofstream f("actors.csv", std::ios::app);
		for (std::vector<Actor>::iterator a = actors.begin(); a != actors.end(); ++a)
		{
		    f << a->sqlInsert() << std::endl;
		}
		f.close();
		std::cout << "Done! " << actors.size() << " entries added to actors.csv" << std::endl;
	    }
	}
	else if (command == "add to scene csv")
	{
	    if (!s_populated)
		std::cout << "Empty List!\n";
	    else{
		std::ofstream f("scenes.csv", std::ios::app);
		for (std::vector<Scene>::iterator s = scenes.begin(); s != scenes.end(); ++s)
		    f << s->sqlInsert() << std::endl;
		f.close();
		std::cout << "Done! " << scenes.size() << " entries addded to scenes.csv" << std::endl;
	    }
	}

	else if (command.find("company list for") != std::string::npos)
	{
		// Get the quoted name
		removeStr(command, '"', true);
		removeStr(command, '"', false);
		totalSceneList_byCompany(command);
	}
	else if (command == "init")
	{
		startSqlServer();
	}
	else if (command == "test ages")
	{
		for (size_t i = 0; i < actors.size(); i++)
		{
			std::vector<Scene> list = actors.at(i).getScenes();
			for (size_t j = 0; j < list.size(); j++)
			{
				testAgeCalculator(actors.at(i), list.at(j));
			}
		}
	}
	else if (command == "clear actor table")
	{
	    std::cout << "Deleting all entries from Actor Table" << std::endl;
	    if (doubleCheck())
		writeSQL_noResponse("DELETE FROM actors"); 
	}
	else if (command == "clear scene table")
	{
	    std::cout << "Deleting all entries from Scene Table" << std::endl;
	    if (doubleCheck())
		writeSQL_noResponse("DELETE FROM scenes");
	}
	else if (command == "run tests")
	{
		runTests();
	}
	else if (command == "sync")
	{
        for (size_t i = 0; i < actors.size(); i++)
		{
			Actor *a = &actors[i];
			std::vector<Scene> list = loadActorsScenes(a->getName());
            for (size_t j = 0; j < list.size(); j++)
			{
				a->addScene(list.at(j));
			}
		}
		syncLists(scenes, actors);
	}
	else if (command == "test naming" || command == "test rename")
	{
		if (s_populated)
			testRename(scenes);
		else
			std::cout << "Load some Scenes into memory first" << std::endl;
	}
	else if (command == "rename files" || command == "commit names" || command == "commit rename" || command == "commit")
	{
		if (doubleCheck())
		{
			for (std::vector<Scene>::iterator s = scenes.begin(); s != scenes.end(); ++s)
			{
				std::cout << "Renaming " << s->filename() << "...\t";
				std::string newName = newFilename(*s);
				if(commitName(*s, newName))
					std::cout << "SUCCESS\n";
				else
					std::cout << "FAILED\n";
			}
		}
	}
	// Run an SQL Query directly from the Command line tool
	else if (command == "sql")
	{
		std::cout << "Running SQL Query:\t" << argv[1];
		customQuery(argv[1]);
	}
	// Make an actor list from the database or (by default:) from the list of scenes in memory.
	
	
	// Query Scenes
	else if (command == "query" || command == "search" || command == "find")
	{
		// Do Database query of Scene Table, get list of scenes back from dbTools, and print them.
		if (argc > 1)
		{			
			std::vector<Scene> templist = searchSceneTable(argv[1]);
			printSceneList(templist);
			templist.clear();
		}
		else
			std::cout << "No Search Term Provided!" << std::endl;
	}
	// Get Actress Bio
	else if (command == "bio" || command == "profile")
	{
		if (argc > 1)
		{
			std::string name = argv[1];
			// Database query or curl request.
			std::cout << "Still working on this one..." << std::endl;
		}
		else
			std::cout << "Need Name to get a bio for!" << std::endl;

	}
	
	else if (command == "help")
	{
		//std::cout << "There is no help. Only Source Code." << std::endl;
		printHelp();
	}
	else
	{
		
		std::cout << "What? Be Sure to separate Operator words with colons [:]" << std::endl;
	}
}


void interactiveMode(std::vector<Scene> &scenes, std::vector<Actor> &actors)
{
	
	add_history("");	// add an entry to start the history
	static std::vector<Scene> opBuffer(0);
	
	if (scenes.size() == 0 && actors.size() == 0)
	    std::cout << "\nWelcome to SceneQuery! Enter 'help' for a list of commands\n" << std::endl;
	else 
	   	std::cout << "\nInteractive Mode Started.\n\n";
	

	bool finished = false;
	// main loop
	while(!finished)
	{
		std::string input; 				// user input buffer.
		input = readline(PROMPT);		// output prompt, and read user input.
		if (input.empty()){continue;}	// allow an empty line
		
		if (input != std::string(history_get(history_length)->line)){
			add_history(input.c_str());											// Add the command to the history buffer.
		}
		std::string copy = input;

		trimWhitespace(copy);													// Remove surrounding whitespace
		std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);		// convert to lowercase
		
		// Check if the user wants to Exit.
		if (copy == "exit" || copy == "quit" || contains(copy, "vim") || contains(copy, "make")){	
			finished = true;	
		}
		else if (copy == "help")	{	
			printHelp();						// Print a help menu if desired.	
		}	
		else
		{
			interpereter2(input, actors, scenes, opBuffer);
		}	
	}

	std::cout << "\nGood Riddance.\n\n";

}
