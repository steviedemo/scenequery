#ifndef __LIST_UTILS_H__
#define __LIST_UTILS_H__

#include "structs.h"
#include <vector>
#define HOME_DIR	"/Users/derby"
#include "Actor-old.h"
#include "Scene.h"
#include <QString>
#include <QStandardItem>
#include <QList>
//sorting
bool 						preOrdered	    (class Actor, 	 class Actor);
bool 						byPath	   		(class Filepath, class Filepath);
bool 						byFilename	    (class Filepath, class Filepath);
// Directory Tools
bool						isNumber		(QString);

// List Searchers
bool			    		inList		    (QList<Actor> &, QString);
Actor*					    findActor		(QList<Actor> &, QString);
Actor*					    ActorSearch 	(QList<Actor> &, QString, int, int);
int					   		SceneSearch	   	(QList<QStandardItem> &, QString, int, int, int);
bool 						SceneSearch		(QList<QStandardItem> &, class Filepath f);
bool						SceneSearch		(QList<QStandardItem> &, class Scene &);

// List Collectors
void 						getActressList						(QList<QStandardItem> &, QList<QStandardItem> &);
//void						getActressList						(QList<QStandardItem> &, QList<QStandardItem> &, runOptions_t);
void						getSceneList						(QList<QStandardItem> &, QList<QStandardItem> &);
//void					 	getSceneList						(QList<QStandardItem>  &, QList<QStandardItem> &, runOptions_t, search_t);
QList<Scene>                getScenesWithActor					(QList<Scene> &scenelist, QString name);
void					 	getScenesWithActor					(QList<QStandardItem> &, QList<QStandardItem> &, QString);
void						retrieveFilmographies				(QList<QStandardItem> &);
void 						addScenesToSecondaryActorProfiles	(QList<QStandardItem> &, QList<QStandardItem> &);

// List Updaters
void						addToList							(QList<QStandardItem> &, class Scene &);
void						addToList							(QList<QStandardItem> &, class Actor &);
QList<QStandardItem>		mergeLists							(QList<QStandardItem> &, QList<QStandardItem> &);
QList<QStandardItem>		mergeLists							(QList<QStandardItem> &, QList<QStandardItem> &);
void						removeDuplicates 					(QList<QStandardItem> &);
void 						removeDuplicates					(QList<QStandardItem> &);
void	 					syncLists							(QList<QStandardItem> &, QList<QStandardItem> &);
void						refreshSceneEntries					(QList<QStandardItem> &, QList<QStandardItem> &);




#endif
