#ifndef __DATABASE_H__
#define __DATABASE_H__
//#include <sqlplus.hh>
#include <pqxx/pqxx>
#include "Filepath.h"   
#include "structs.h"
#define HOST "localhost"
#define ACTOR_DB std::string("actors")
#define SCENE_DB std::string("scenes")
#define FILM_DB  std::string("filmographies")
#define USERNAME "root"
#define PASSWORD ""
#define DB_NAME "derby"
#define HOST    "localhost"
#define PORT    5432

//#ifdef __APPLE__ 
#define DB_CREDS "dbname=derby user=derby password=smashed hostaddr=127.0.0.1 port=5432"
//#else
#define SCENE_TABLE 		0
#define ACTOR_TABLE			1
#define FILMOGRAPHY_TABLE 	2
#define THUMBNAIL_TABLE 	3
#define HEADSHOT_TABLE 		4

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>


#define ADB_TABLE 	"CREATE TABLE IF NOT EXISTS ACTORS("\
					"ID				smallserial 	primary key,"\
					"NAME 			text 			unique not null,"\
					"ALIASES		text,"\
					"BIRTHDAY		date,"\
					"CITY			text,"\
					"COUNTRY		text,"\
					"ETHNICITY		text,"\
					"HEIGHT			integer,"\
					"WEIGHT			integer,"\
					"MEASUREMENTS	text,"\
					"HAIR			text,"\
					"EYES			text,"\
					"TATTOOS		text,"\
					"PIERCINGS		text,"\
					"PHOTO			text)"
#define SDB_TABLE	"CREATE TABLE IF NOT EXISTS SCENES("\
					"ID				serial			primary key,"\
					"FILENAME		text			not null,"\
					"FILEPATH		text			not null,"\
					"TITLE			text,"\
					"COMPANY		text,"\
					"SERIES			text,"\
					"SCENE_NO		integer,"\
					"RATING			text,"\
					"SIZE			float8,"\
					"LENGTH			float8,"\
					"WIDTH			integer,"\
					"HEIGHT			integer,"\
					"ADDED			date,"\
					"CREATED		date,"\
					"ACCESSED		date,"\
					"ACTOR1			text,"\
					"AGE1			integer,"\
					"ACTOR2			text,"\
					"AGE2			integer,"\
					"ACTOR3			text,"\
					"AGE3			integer,"\
					"ACTOR4			text,"\
					"AGE4			integer,"\
					"URL			text,"\
					"TAGS			text)"
// Store entries from online filmographies
#define FDB_TABLE	"create table if not exists filmographies("\
					"ID				serial			primary key,"\
					"actor			text			not null,"\
					"title			text			not null,"\
					"company		text			not null,"\
					"year			integer			not null,"\
					"scene			integer,"\
					"tags			text)"
#define THUMBNAIL_DB	"create table if not exists thumbnails("\
					"ID 			serial 			primary key,"\
					"SCENEID 		integer			not null,"\
					"FILENAME		text			not null,"\
					"DATEADDED		text,"\
					"SCENEPATH		text,"\
					"SCENEFILE 		text,"\
					"IMAGE			OID)"
#define HEADSHOT_DB	"create table if not exists thumbnails("\
					"ID 			serial 		primary key,"\
					"FILENAME		text			not null,"\
					"DATEADDED		text,"\
					"NAME 			text,"\
					"IMAGE			bytea)"
// Low Level Routines
void						startSqlServer			(void);
bool						createTable				(int);
int	 						SceneSize				(pqxx::result::const_iterator &);
int 						ActorSize				(pqxx::result::const_iterator &);
double						sceneCompleteness		(pqxx::result::const_iterator &);
double						actorCompleteness		(pqxx::result::const_iterator &);

bool checkField(QSqlField field);

// Record Maintenance
void						purgeSceneTable			(void);
void						purgeActorTable			(void);
bool						updateFilename			(std::string, std::string, std::string);
void						customQuery				(std::string s);
void 						writeSQL_noResponse		(std::string sql);

// Search Records
std::vector<class Scene> 	searchSceneTable		(std::string searchterm);
bool						isSceneStored			(Filepath);
bool						isSceneStored			(class Scene &);
bool						isActorStored			(std::string);
bool						isActorStored			(class Actor &);
// Add Records
bool						storeFilm				(class Scene &);
bool						storeScene				(class Scene &);
bool						storeActor				(class Actor &);
bool						storeFilmList			(std::vector<class Scene>);
bool						storeSceneList			(std::vector<class Scene> &);
bool						storeActorList			(std::vector<class Actor> &, bool verbose = false);
// Retrieve Records
class Scene 				loadScene 				(Filepath);
void 						loadScenes				(std::vector<class Scene> &);
class Actor					loadActor				(std::string name);
void						loadActors				(std::vector<class Actor> &);
std::vector<class Scene>	loadActorsScenes		(std::string);
void						loadActorsScenes		(std::vector<class Scene> &, std::string);
std::vector<class Scene> 	loadScenesWhere			(std::string sql_search);
bool						loadActorsFilmography	(std::vector<class Scene> &, std::string);

Actor   getActorFromRecord(QSqlRecord *);
Scene   getSceneFromRecord(QSqlRecord *);
bool    checkField(QSqlField, int);
// Higher Level Data Extraction
std::vector<string_count_t> getCompanyCounts		(void);

// CSV Functions
void 						createActorCSV 			(std::vector<class Actor> list, std::string filename);
void 						createSceneCSV 			(std::vector<class Scene> list, std::string filename);
std::vector<class Actor> 	actorListFromCSV		(std::string filename);
std::vector<class Scene> 	sceneListFromCSV 		(std::string filename);
#endif
