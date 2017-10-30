#ifndef __OUTPUT_H__
#define __OUTPUT_H__
#include "structs.h"
#define LOG_FILE	"./data/error_log.txt"
//#define TEST_OUTPUT
#define logError(e);		recordError(__FILE__, __FUNCTION__, __LINE__, e);
#define logFileError(e, f);	recordError(__FILE__, __FUNCTION__, __LINE__, e, f);
//#define logError(e); std::cerr << "\n!!! Error in " << __FILE__ << " in " << __FUNCTION__ << " @ " << __LINE__ << ": " << e << std::endl << std::endl;
#define note()		 std::cout << __FILE__ << " in " << __FUNCTION__ << " @ " << __LINE__ << std::endl;
 #ifdef TEST_OUTPUT 
 #define debug(); std::cout <<  __FILE__ << " in " << __FUNCTION__ << " at line " << __LINE__ << std::endl;
 #else 
#define debug();
 #endif
#define traceLine();    omptraceLine(__LINE__, __FUNCTION__);
#define track(s);   ompDebug(__LINE__, __FUNCTION__, s);
void 	recordError 		(const char *, const char *, int, std::string errorMsg, std::string filename = "");
void 	log					(std::string s);
void	omptraceLine		(char *, char *, int);
void	ompTrack			(char *, char *, int, std::string);
void	printBio			(bio_t &bio);
void 	printSceneList		(std::vector<class Scene> &s);
void	saveSceneList		(std::vector<class Scene> &s, std::string);
void 	printActressList	(std::vector<class Actor> &a);
void 	printDetails		(std::vector<class Scene> &s, std::vector<class Actor> &a, std::string dir);
void 	printUsage			(void);
void	toTextFile			(std::vector<class Filepath> &, std::string, bool);
void 	toTextFile			(std::vector<class Scene> , std::string filename);
void 	outputList			(std::vector<class Actor> &a, std::string name = "Actors.txt");
void 	outputList			(std::vector<class Scene> &, std::string name = "Scenes.txt");
#endif
