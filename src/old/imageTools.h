#ifndef __IMAGE_TOOLS_H__
#define __IMAGE_TOOLS_H__
#include <vector>
#define THUMBNAIL_PATH	"/Users/derby/SceneQuery/Data/Thumbnails/"
#define HEADSHOT_PATH 	"/Users/derby/SceneQuery/Data/Headshots/"
// Image Handling
Filepath					downloadHeadshot		(std::string name);
Filepath 					downloadHeadshot 		(class Actor &);
void						downloadHeadshots	 	(std::vector<class Actor> &);
bool						headshotExists			(class Actor &);
bool						headshotExists			(std::string name);

bool						thumbnailExists			(class Scene &);
void 						generateThumbnails		(class Scene &);
void 						generateThumbnails 		(std::vector<class Scene> &);

// Database Tools
#endif