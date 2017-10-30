#ifndef __CURL_REQUESTS_H__
#define __CURL_REQUESTS_H__

#include "Filepath.h"   
#include "structs.h"
#include <vector>
// Getting Request Strings & Page HTML
std::string					getFreeonesURL			(std::string);
std::string					getIAFDURL				(std::string);
std::string 				getIAFDhtml				(std::string);
std::string					fetchHTML				(std::string);

// Low Level HTML Parsing
std::string 				getMatch				(std::string, std::string);
std::string 				getData					(std::string, const char *);
void 						getSelectLines			(std::vector<std::string> &, std::vector<std::string> &, std::string);
std::vector<std::string>	returnPartOfHTML		(std::string, std::string, int);
Filepath					getBioPicFromHTML		(std::string, std::string);
std::vector<std::string> 	htmlToVector			(std::string);
std::string 				getIAFDBioLine			(std::vector<std::string> &html);
bool fetchHeightAndWeight(std::string name, height_t &h, int &weight);


// Higher Level Parsing
bool						fetchFreeonesBio 		(std::string html, bio_t &bio);
bool 						fetchIAFDBio			(std::vector<std::string> &, bio_t &);
std::vector<std::string>	getLinks				(std::vector<std::string>);
content_t					stripTags				(std::string);
std::string					getContent				(std::string);
std::vector<class Scene>	getOnlineFilmography	(std::string);
class Scene					getScene				(std::string);

// API Level Calls
std::vector<class Scene>	fetchFilmography		(std::string name);
bio_t						fetchFreeonesBio		(std::string name);
bio_t 						fetchIAFDBio			(std::string name);
bio_t 						fetchBio 				(std::string name);
#endif

