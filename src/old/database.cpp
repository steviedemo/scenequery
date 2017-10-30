#include <algorithm>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <string>
#include <stdexcept>
#include <vector>
#include "Actor-old.h"
#include "Filepath.h"
#include "Scene.h"
#include "lists.h"
#include "stringTools.h"
#include "systemInterface.h"
#include "database.h"


#include <pqxx/pqxx>
#include "qsqldbhelper.h"

#define DEBUG
#define logError(arg);	std::cout << "\nError @ line " << __LINE__ << " in fxn " << __FUNCTION__ << " in dbTools.cpp:\n\t" << arg << std::endl;
#define traceLine()			std::cout << __LINE__ << " in " << __FUNCTION__ << " in dbTools.cpp" << std::endl;
#ifdef DEBUG
#define testOutput(s);	std::cout << "\n@ " << __LINE__ << " in " << __FUNCTION__ << " (dbTools.cpp):\t" << s << std::endl;
#endif
#define BUFFSIZE 4096
#define NUM_THREADS 32

#define QSQL

std::string systemCall(std::string);


// Utilities

double sceneCompleteness(pqxx::result::const_iterator &I){
    double ratio = (double)SceneSize(I);
    ratio /= SCENE_SIZE;
    return ratio;
}

double actorCompleteness(pqxx::result::const_iterator &I){
    double ratio = (double)ActorSize(I);
    ratio /= ACTOR_SIZE;
    return ratio;
}

int SceneSize(pqxx::result::const_iterator &I){
    int count = 0;
    try{
		for (int i = 1; i < 24; i++){
		    if (!I[i].is_null())
			++count;
		}
    }catch(std::exception &e){logError(e.what());}
    return count;
}

int ActorSize(pqxx::result::const_iterator &I){
    int count = 0;
    try{
		for (int i = 1; i < 14; i++){
		    if (!I[i].is_null())
			++count;
		}
    }catch(std::exception &e){logError(e.what());}
    return count;
}


//---------------------------------------------------------------------------------------------------------
//			LOW LEVEL ROUTINES
//---------------------------------------------------------------------------------------------------------

void startSqlServer(void)
{
    std::cout << "Attempting to start/restart Posgres Server - Output:" << std::endl;
    //std::string output = systemCall("postgres -D /usr/local/pgsql/data >logfile 2>&1 &");
#ifdef __APPLE__
    std::string output = systemCall("pg_ctl -D /usr/local/var/postgres -l /usr/local/var/postgres/server.log start");
#else
    std::string output = systemCall("sudo service postgresql restart");
#endif
    std::cout << output << std::endl;
}



bool createTable(int table_id){
    bool success = false;
    std::string sql("");
    if (table_id == ACTOR_TABLE)
    	sql = std::string(ADB_TABLE);
    else if (table_id == FILMOGRAPHY_TABLE)
    	sql = std::string(FDB_TABLE);
    else if (table_id == SCENE_TABLE)
    	sql = std::string(SDB_TABLE);
    else if (table_id == THUMBNAIL_TABLE)
    	sql = std::string(THUMBNAIL_DB);
    else if (table_id == HEADSHOT_TABLE)
    	sql = std::string(HEADSHOT_DB);
    if (!sql.empty())
    {
	    try{
			pqxx::connection c(DB_CREDS);
			if (!c.is_open()){	logError("Error Opening Database Connection");	}
			else{
			    std::cout << "Database Connection Successfully Established."<< std::endl;
			    pqxx::work w(c);
			    w.exec(sql.c_str());
			    w.commit();
			    success = true;
			    testOutput("Created Scene Table in Database");
			    c.disconnect();
			}
	    }catch (const std::exception &e){logError(std::string(e.what()));}
    }
    return success;
}


void writeSQL_noResponse(std::string sql){
    try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){ std::cerr << "Error opening connection" << std::endl;}
		else{
		    pqxx::nontransaction n(c);
		    n.exec(sql.c_str());
		    n.commit();
		}
	}catch(std::exception &e){std::cerr<<e.what()<<std::endl;}
}

void customQuery(std::string query){
	std::string creds("");
	bool validDb = false;
	if (query.find("ACTOR") != std::string::npos)
	{
		validDb = true;
		creds = std::string(DB_CREDS);
	}
	else if(query.find("SCENE") != std::string::npos)
	{
		validDb = true;
		creds = std::string(DB_CREDS);
	}
	if (validDb){
		try{	
			pqxx::connection c(creds);
			if (!c.is_open())	{	logError("Error opening database");	}
			else{
				pqxx::nontransaction N(c);
				// Execute the Query
				pqxx::result R(N.exec(query));
				// Check if anything is in the set of results
				if (R.size() > 0)
				{
					std::cout << "Results: "<< std::endl;
					for(pqxx::result::const_iterator I = R.begin(); I != R.end(); ++I)
					{
                        for (size_t i = 0; i < I.size(); i++)
						{
                            std::cout << I[(int)i].as<std::string>() << std::endl;
						}
						std::cout << std::endl;
					}
				}else{	std::cout << "No Results Found." << std::endl; }
			}
		}catch(std::exception &e){logError(std::string(e.what()));}
	}else{	logError("Unrecognized Database/Table Name.");	}
}

bool checkField(QSqlField field)        {   return (field.isValid() && !field.isNull() && !field.isAutoValue());    }
bool checkField(QSqlRecord* r, int i)   {   return (r->field(i).isValid() && !r->field(i).isNull() && !r->field(i).isAutoValue());  }

Scene getSceneFromRecord(QSqlRecord* r)
{
    Scene s;
    if (!r->isNull() && !r->isEmpty())
    {
        try{
            if (checkField(r->field(1)))    {   s.setFilename(r->field(1).setType(QVariant::ByteArray)); }
            if (checkField(r->field(2)))    {   s.setPath   (r->field(2).setType(QVariant::ByteArray));  }
            if (checkField(r->field(3)))    {   s.setTitle  (r->field(3).setType(QVariant::ByteArray));  }
            if (checkField(r->field(4)))    {   s.setCompany(r->field(4).setType(QVariant::ByteArray));  }
            if (checkField(r->field(5)))    {   s.setSeries (r->field(5).setType(QVariant::ByteArray));  }
            if (checkField(r->field(6)))    {   s.setSceneNo(r->field(6).setType(QVariant::Int));        }
            if (checkField(r->field(7)))    {   s.setRating (r->field(7)).setType(QVariant::ByteArray);  }
            if (checkField(r->field(8)))    {   s.setSize   (r->field(8)).setType(QVariant::Double);     }
            if (checkField(r->field(9)))    {   s.setLength (r->field(9).setType(QVariant::Double));     }
            if (checkField(r->field(10)))   {   s.setWidth  (r->field(10).setType(QVariant::Int));       }
            if (checkField(r->field(11)))   {   s.setHeight (r->field(11).setType(QVariant::Int));       }
            if (checkField(r->field(12)))   {   s.setDateAdded      (textDateToStruct(r->field(12).setType(QVariant::ByteArray)));  }
            if (checkField(r->field(13)))   {   s.setReleaseDate    (textDateToStruct(r->field(13).setType(QVariant::ByteArray)));  }
            if (checkField(r->field(14)))   {   s.setDateAccessed   (textDateToStruct(r->field(14).setType(QVariant::ByteArray)));  }
            int actorNum = 0;
            for (i = 15; i < 22; i += 2)
            {
                if (checkfield(r->field(i)))    {   s.addActor(r->field(i).setType(QVariant::ByteArray));                   }
                if (checkfield(r->field(i+1)))  {   s.setAgeOfActorNum(actorNum++, r->field(i+1).setType(QVariant::Int));   }
            }
        } catch (std::exception &e) {
            qDebug() << "Error Creating Scene Object from record.";
        }
    }
    return s;
}

Actor getActorFromRecord(QSqlRecord *r)
{
    Actor a;
    if (!r->isNull() && !r->isEmpty())
    {
        try{
            if (checkField(r, 1))  {
                a.setName(r->field(1).setType(QVariant::ByteArray));
            } else {
                return a;
            }
            if (checkField(r, 2))   {   a.setAliases(r->field(2).setType(QVariant::ByteArray)); }
            if (checkField(r, 3))   {   a.setBirthday(r->field(3).setType(QVariant::ByteArray)); }
            if (checkField(r, 4))   {   a.setCity(r->field(4).setType(QVariant::ByteArray)); }
            if (checkField(r, 5))   {   a.setNationality(r->field(5).setType(QVariant::ByteArray)); }
            if (checkField(r, 6))   {   a.setEthnicity(r->field(6).setType(QVariant::ByteArray)); }
            if (checkField(r, 7))   {   height_t h(r->field(7).setType(QVariant::Int)); a.setHeight(h); }
            if (checkField(r, 8))   {   a.setWeight(r->field(8).setType(QVariant::Int));    }
            if (checkField(r, 9))   {   a.setMeasurements(r->field(9).setType(QVariant::ByteArray)); }
            if (checkField(r, 10))   {   a.setHair(r->field(10).setType(QVariant::ByteArray)); }
            if (checkField(r, 11))   {   a.setEyes(r->field(11).setType(QVariant::ByteArray)); }
            if (checkField(r, 12))   {   a.setTattoos(r->field(12).setType(QVariant::ByteArray)); }
            if (checkField(r, 13))   {   a.setPiercings(r->field(13).setType(QVariant::ByteArray)); }
        } catch (std::exception &e) {
            qDebug() << "Error getting Actor from database Record";
        }
    }
    return a;
}


Scene sqlToScene(pqxx::result::const_iterator &i){

    Scene s;
    std::string temp("");
    try{
	    if (!i[1].is_null())
		s.setFilename(i[1].as<std::string>());
	    if (!i[2].is_null())
		s.setPath(i[2].as<std::string>());
	    if (!i["title"].is_null())
		s.setTitle(i[3].as<std::string>());
	    if (!i["company"].is_null())
		s.setCompany(i[4].as<std::string>());
	    if (!i["series"].is_null())
		s.setSeries(i[5].as<std::string>());
	    if (!i[6].is_null())
		s.setSceneNo(i[6].as<int>());
	    if (!i["rating"].is_null())
		s.setRating(i[7].as<std::string>());
	    if (!i["size"].is_null())
		s.setSize((double)i[8].as<float>());
	    if (!i["length"].is_null())
		s.setLength((double)i[9].as<float>());
	    if (!i["width"].is_null())
		s.setWidth(i[10].as<int>());
	    if (!i["height"].is_null())
		s.setHeight(i[11].as<int>());
	    if (!i["added"].is_null())
		s.setDateAdded(textDateToStruct(i[12].as<std::string>()));
	    if (!i["created"].is_null())
		s.setReleaseDate(textDateToStruct(i[13].as<std::string>()));
	    if (!i[14].is_null())
		s.setDateAccessed(textDateToStruct(i[14].as<std::string>()));
	    if (!i["actor1"].is_null()){
		s.addActor(i[15].as<std::string>());
		if(!i["age1"].is_null())
		    s.setAgeOfActorNum(0, i[16].as<int>());
	    }
	    if (!i["actor2"].is_null()){
		s.addActor(i[17].as<std::string>());
		if(!i["age2"].is_null())
		    s.setAgeOfActorNum(1, i[18].as<int>());
	    }
	    if (!i[19].is_null()){
		s.addActor(i[19].as<std::string>());
		if(!i[20].is_null())
		    s.setAgeOfActorNum(2, i[20].as<int>());
	    }
	    if (!i[21].is_null()){
		s.addActor(i[21].as<std::string>());
		if(!i[22].is_null())
		    s.setAgeOfActorNum(3, i[22].as<int>());
	    }
	    if (!i[23].is_null())
		s.setSourceURL(i[23].as<std::string>());
	}catch(std::out_of_range &e){logError(e.what());}
    return s;
}

Actor sqlToActor(pqxx::result::const_iterator &i){
    std::string name(i[1].as<std::string>());
    Actor a(name);
    try{
	    if (!i["aliases"].is_null()) { a.setAliases(i[2].as<std::string>()); }
	    if (!i[3].is_null()) { a.setBirthday(i[3].as<std::string>()); }
	    if (!i["city"].is_null()) { a.setCity(i[4].as<std::string>()); }
	    if (!i[5].is_null()) { a.setNationality(i[5].as<std::string>()); }
        if (!i[6].is_null()) { a.setEthnicity(i[6].as<std::string>()); }
	    if (!i[7].is_null()){
		height_t h(atoi((i[7].as<std::string>()).c_str()));
		a.setHeight(h);
	    }
	    if (!i[8].is_null()) { a.setWeight(i[8].as<int>()); }
	    if (!i[9].is_null()) { a.setMeasurements(i[9].as<std::string>()); }
	    if (!i[10].is_null()) { a.setHair(i[10].as<std::string>()); }
	    if (!i[11].is_null()) { a.setEyes(i[11].as<std::string>()); }
	    if (!i[12].is_null()) { a.setTattoos(i[12].as<std::string>()); }
	    if (!i[13].is_null()) { a.setPiercings(i[13].as<std::string>()); }
	}catch(std::out_of_range &e){logError(e.what());}
    return a;
}

//---------------------------------------------------------------------------------------------------------
//			AUTOMATED MAINENANCE ROUTINES
//---------------------------------------------------------------------------------------------------------


// Needs a connection to have been established first
void deleteRecord(pqxx::connection &c, Filepath f)
{
    std::string sql_delete = "DELETE FROM SCENES WHERE FILENAME LIKE " + sqlSafe(f.filename()); 
    pqxx::nontransaction N_delete(c);
    N_delete.exec(sql_delete.c_str());
    N_delete.commit();
}

void smartPurge(pqxx::connection &c, pqxx::result &R)
{
	unsigned int idx = 0;
	try{
		if (!R.empty())
		{
			omp_set_dynamic(0);
			#pragma omp parallel for num_threads(NUM_THREADS)
			for (size_t i = 0; i < R.size(); ++i)
			{
				#pragma omp critical 
				{
				    ++idx;
				}
				pqxx::result::const_iterator I = R.begin() + i;
				if (!I[1].is_null() && !I[2].is_null() && !I["title"].is_null() && !I["actor1"].is_null())
				{
					// Get Necessary Fields out of the Record entry
					std::string title(I["title"].as<std::string>());
					std::string actor(I["actor1"].as<std::string>());
					Filepath f(I[2].as<std::string>(), I[1].as<std::string>());

					// First Check whether file with same name as in this record exists on the disk
					if (!f.exists())
					{
						std::string newFilename("");
						// If not, check if a file containing the actor's name and the title exists in the same folder.
						if (fileWasRenamed(f.filepath(), actor, title, newFilename))
						{
							Filepath newFile	(f.filepath(), newFilename);
							Scene newScene		(newFile);

							#pragma omp critical
							{
								std::cout << idx << '/' << R.size() << ":\t";
								std::cout << f.filename() << "--->" << newFilename << std::endl;
								storeScene(newScene);	// Store the new Record
								deleteRecord(c, f);
							}
						}
						else
						{
							#pragma omp critical
							{
								std::cout << idx << "/" <<R.size()<< ":\tDeleting ";
								std::cout << f.fullpath() << std::endl;
								deleteRecord(c, f);
							}
						}
					}	// if !exists
				}// if fields aren't null
			}// for loop
		} // if results aren't empty
	}catch(std::exception &e){
		logError(e.what());
	}
}


unsigned int standardPurge(pqxx::connection &c, pqxx::result &R_collect)
{
	 unsigned int idx = 0, deleted = 0;
    // Go through the records and check each for existance
	try{
		if (R_collect.size() > 0)
		{
			for (pqxx::result::const_iterator I = R_collect.begin(); I != R_collect.end(); ++I)
		    {
		    	++idx;
		    	if (I[1].is_null() || I[2].is_null())
		    		std::cout << ++idx << " - Empty Result\n";
		    	else
		    	{
					Filepath f(I[2].as<std::string>(), I[1].as<std::string>());

					if (!f.exists())// Delete Record from table.
					{
					    std::string sql_delete = "DELETE FROM SCENES WHERE FILENAME LIKE " + sqlSafe(f.filename()); 
					 	std::cout << idx << "/" <<R_collect.size()<< ":\tDeleting " << f.fullpath() << std::endl;
					 	pqxx::nontransaction N_delete(c);
					    N_delete.exec(sql_delete.c_str());
					    N_delete.commit();
					    ++deleted;
					}
				}
		    }
			std::cout << "Purge Complete! Deleted " << deleted << " Outdated Records out of " << R_collect.size() << " total records" << std::endl;
		}
		else
			std::cout << "No Records Found" << std::endl;
	}catch(std::exception &e){logError(e.what());}	    
	return deleted;
}
// purgeSceneTable:		Remove any records for files that no longer exist with the recorded name/location.
void purgeSceneTable(void){
    std::cout << "Purging Scene Database of Moved, Deleted or Renamed Files" << std::endl;
    try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){logError("Can't open Scene Database");}
		else{
		    std::string sql_collect("SELECT * from SCENES");
		    pqxx::nontransaction N_collect(c);
		    pqxx::result R_collect(N_collect.exec(sql_collect.c_str()));	// Execute the Query and get Results
		    N_collect.commit();
			//standardPurge(R_collect);
			smartPurge(c, R_collect);		   
		    c.disconnect();
		}
    }catch(const std::exception &e){logError(std::string(e.what()));}
}

// go through each actor in the actors table, search for their name in the 4 actor fields of the scenes table, and if not one entry comes back matching their name, delete their entry. 
void purgeActorTable(void){
    try{
		std::vector<std::string> namesToRemove(0);
		pqxx::connection c(DB_CREDS);
		if(!c.is_open()){logError("Error opening actor database");}
		else 
		{
		    std::string sql_collect("SELECT name FROM actors");
		    pqxx::nontransaction N_collect(c);
		    pqxx::result R_collect(N_collect.exec(sql_collect.c_str()));
		    N_collect.commit(); 
		    // Loop over all collected entries. Search the scene Table for each actor name, and add any without any hits from the scene table to the list of entries to remove.
		    for (pqxx::result::iterator I = R_collect.begin(); I != R_collect.end(); ++I)
		    {
				std::string name = I[0].as<std::string>();
				std::string sql_search("SELECT id FROM scenes WHERE ");
				sql_search += "actor1 = " + sqlSafe(name) + " OR actor2 = " + sqlSafe(name) + " OR actor3 = " + sqlSafe(name) + " OR actor4 = " + sqlSafe(name) + ";";
				pqxx::nontransaction N_search(c);
				pqxx::result R_search(N_search.exec(sql_search.c_str()));
				N_search.commit();
				if (R_search.size() < 1){
				    namesToRemove.push_back(name); 
				}
		    }
		    // done looping over all entries. loop over entries to remove.
		    for (std::vector<std::string>::iterator i = namesToRemove.begin(); i != namesToRemove.end(); ++i)
		    {
				std::cout << "Purging " << *i << " from the Actor Table." << std::endl;
				std::string sql_delete("DELETE FROM actors WHERE name = " + sqlSafe(*i) + ";");
				pqxx::nontransaction N_delete(c);
				N_delete.exec(sql_delete.c_str());
				N_delete.commit();
		    }
		    std::cout << "\n" << namesToRemove.size() << " were Removed from the Actor Table.\n\n"; 
		}
    }catch(std::exception &e){logError(e.what());}
}

// Used in conjunction with file renaming utility (found in tools.cpp) to update records when a file is renamed via the tool.
bool updateFilename(std::string path, std::string oldName, std::string newName){
    bool success = false;
    try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){logError("Error opening connection\n");}
		else
		{
		    std::string sql_search = "SELECT id FROM scenes WHERE filename = " + sqlSafe(oldName) + " AND filepath = " + sqlSafe(path) + ";";
		    pqxx::nontransaction N_search(c);
		    pqxx::result R(N_search.exec(sql_search.c_str()));
		    N_search.commit();
		    if (R.size() > 0)
		    {
				std::string sql_update("UPDATE scenes SET filename = "+sqlSafe(newName)+" WHERE filename = "+sqlSafe(oldName)+" AND filepath = "+ sqlSafe(path) +";"); 
				pqxx::nontransaction N_update(c);
				N_update.exec(sql_update.c_str());
				N_update.commit();
				success = true;
		    }
		}
    }
    catch(std::exception &e){logError(e.what());}
    return success;
} 

//---------------------------------------------------------------------------------------------------------
//			RECORD ADDING METHODS
//---------------------------------------------------------------------------------------------------------
bool storeSceneListV2(std::vector<Scene> &sceneList){
    bool success = false;
    QSqlDBHelper dbhelper("QPSQL");
    QSqlDatabase* db = dbhelper.connect(HOST, DB_NAME, USERNAME, "smashed");
    if (db->open())
    {
        unsigned int idx = 0, updated = 0, added = 0;
        size_t num_scenes = sceneList.size();
        qDebug() << "Connected to the Scene Database. ";
        qDebug() << "Adding " << num_scenes << " Scenes.";
        for (auto const& S : sceneList)
        {
            QString queryString;
            QStringList list;
            if (S->size() > 2)
            {

                queryString("SELECT FROM SCENES WHERE (TITLE LIKE ? AND ACTOR1 LIKE ? AND RELEASED LIKE ?)");
                list << sqlSafe(S->getTitle()) << sqlSafe(S->getFirstActor()) << sqlSafe(S->getReleaseDate());
                QSqlQuery *query = dbhelper.sceneQuery(queryString, list);
                // Scene isn't in DB
                try{
                    if (query->result()->isActive() == false)
                    {
                        if (S->sqlInsert(queryString, list))
                        {
                            ++added;
                            dbhelper.sceneQuery(queryString, list);
                            std::cout << addTabs(std::to_string(++idx) + "/" + std::to_string(num_scenes), 2) << "Added:\t" << S->filename() << std::endl;
                        }
                    }
                    // Scene IS in DB, check if we need to update the filename.
                    else
                    {

                    }
                } catch (std::exception &e) {
                    logError(e.what());
                }

            }
        }
    }
}

// Attempt to use First actor, Title & Release Date as critera to determine if a scene is in the database.
// This version has an added feature in that it attempts to distinguish between a file having been deleted vs. a file who's name has been changed (ie, added tags or rating).
bool storeSceneListV2(std::vector<Scene> &sceneList){
    bool success = false;
    try{
	pqxx::connection c(DB_CREDS);
	if(!c.is_open()){	logError("Error Opening Scene Database");	}
	else {
		unsigned int idx = 0, updated = 0, added = 0;
		size_t num_scenes = sceneList.size();		
	    std::cout << "Connected to the Database.\nAdding " << num_scenes << " Scenes." << std::endl;
	    for (std::vector<Scene>::iterator S = begin(sceneList); S != end(sceneList); ++S)
	    {
			try
			{
			    if (S->size() > 2)
			    {
					std::string sql_search("SELECT FROM SCENES WHERE (TITLE LIKE " + sqlSafe(S->getTitle()) + " AND ACTOR1 LIKE " + sqlSafe(S->getFirstActor()) + " AND RELEASED LIKE " + sqlSafe(S->getReleaseDate()) + ")");
					pqxx::nontransaction N_search(c);
					pqxx::result R(N_search.exec(sql_search.c_str()));
					N_search.commit();
					// scene not in database
					if (R.size() < 1)
					{
					    try{
							++added;
						    pqxx::nontransaction N_insert(c);
							N_insert.exec(S->sqlInsert().c_str());
							N_insert.commit();
							std::cout << addTabs(std::to_string(++idx) + "/" + std::to_string(num_scenes), 2) << "Added:\t" << S->filename() << std::endl;
					    }catch(std::exception &e){logError(e.what());}
					}
					// scene is in database
					else
					{

					    pqxx::result::const_iterator I = R.begin();
					    // If the filename differs or if it was moved, update the record to reflect the scene's new name/location.
					    // If the record in RAM has more data items then that in the table, update it to add the new data.
					    if (S->filename() != I[1].as<std::string>() || S->filepath() != I[2].as<std::string>() || S->size() > SceneSize(I))
					    {
					   		try{
							    std::string updateSql(S->sqlUpdate());						    
							    if (updateSql != "")
							    {
									++updated;
									pqxx::nontransaction N_update(c);
									N_update.exec(updateSql.c_str());
									N_update.commit();
									std::cout << addTabs(std::to_string(++idx) + "/" + std::to_string(num_scenes), 2) << "Updated\t" << S->filename() << std::endl;
								}
							}catch(std::exception &e){logError(e.what());}
					    }
					}
			    }
			}catch(std::exception &e){logError(std::string(e.what()) + ": " + S->filename());}
	    }
		std::cout << "\n\n" << added << " Scenes Added\n" << updated << " Scenes Updated\n" << idx << "/" << num_scenes << " scenes in the list were recorded as new in the database.\n\n";
	    c.disconnect();
	    success = true;
	}
    }catch(const std::exception &e){logError(e.what());}
    return success;
}

bool storeSceneList(std::vector<Scene> &sceneList){
    bool success = false;
    try{
	pqxx::connection c(DB_CREDS);
	if(!c.is_open()){	logError("Error Opening Scene Database");	}
	else {
		unsigned int idx = 0, updated = 0, added = 0;
		size_t num_scenes = sceneList.size();		
	    std::cout << "Connected to the Database.\nAdding " << num_scenes << " Scenes." << std::endl;
	    for (std::vector<Scene>::iterator S = begin(sceneList); S != end(sceneList); ++S)
	    {
			try
			{
			    if (S->size() > 2)
			    {
					std::string sql_search("SELECT FROM SCENES WHERE (FILENAME LIKE " + sqlSafe(S->filename()) + ")");
					pqxx::nontransaction N_search(c);
					pqxx::result R(N_search.exec(sql_search.c_str()));
					N_search.commit();
					// scene not in database
					if (R.size() < 1)
					{
					    try{
							++added;
						    pqxx::nontransaction N_insert(c);
							N_insert.exec(S->sqlInsert().c_str());
							N_insert.commit();
							std::cout << addTabs(std::to_string(++idx) + "/" + std::to_string(num_scenes), 2) << "Added:\t" << S->filename() << std::endl;
					    }catch(std::exception &e){logError(e.what());}
					}
					// scene is in database
					else
					{
					    pqxx::result::const_iterator I = R.begin();
					    if (S->size() > SceneSize(I))
					    {
							try{
							    std::string updateSql(S->sqlUpdate());						    
							    if (updateSql != "")
							    {
									++updated;
									pqxx::nontransaction N_update(c);
									N_update.exec(updateSql.c_str());
									N_update.commit();
									std::cout << addTabs(std::to_string(++idx) + "/" + std::to_string(num_scenes), 2) << "Updated\t" << S->filename() << std::endl;
								}
							}catch(std::exception &e){logError(e.what());}
					    }
					}
			    }
			}catch(std::exception &e){logError(std::string(e.what()) + ": " + S->filename());}
	    }
		std::cout << "\n\n" << added << " Scenes Added\n" << updated << " Scenes Updated\n" << idx << "/" << num_scenes << " scenes in the list were recorded as new in the database.\n\n";
	    c.disconnect();
	    success = true;
	}
    }catch(const std::exception &e){logError(e.what());}
    return success;
}

bool storeFilmList(std::vector<Scene> sceneList)
{
	std::cout << "Storing Filmography Records..." << std::endl;
	bool success = false;
try{
	pqxx::connection c(DB_CREDS);
	if(!c.is_open()){	logError("Error Opening Scene Database");	}
	else {
		unsigned int idx = 0, updated = 0, added = 0;
		size_t num_scenes = sceneList.size();		
	    std::cout << "Adding " << num_scenes << " Filmography entries to the Database...\t";
	    for (std::vector<Scene>::iterator S = begin(sceneList); S != end(sceneList); ++S)
	    {
			try
			{
			    if (S->size() > 2)
			    {
			    	// Check if record is in database before storing it to avoid duplicates.
					std::string sql_search("select from " + FILM_DB + " WHERE (ACTOR LIKE " + sqlSafe(S->getActorNumber(0)) + " AND TITLE = " + sqlSafe(S->getTitle()) +  " AND COMPANY = " + sqlSafe(S->getCompany()) + ")");
					pqxx::nontransaction N_search(c);
					pqxx::result R(N_search.exec(sql_search.c_str()));
					N_search.commit();
					// scene not in database
					if (R.size() < 1)
					{
					    try{
							++added;
						    pqxx::nontransaction N_insert(c);
							N_insert.exec(S->shortSqlInsert().c_str());
							N_insert.commit();
							std::cout << addTabs(std::to_string(++idx) + "/" + std::to_string(num_scenes), 2) << "Added:\t" << S->getTitle() << std::endl;
					    }catch(std::exception &e){logError(e.what());}
					}
					// scene is in database
			    }
			}catch(std::exception &e){logError(std::string(e.what()) + ": " + S->filename());}
	    }
		std::cout << added << " Entries Added.\n" << updated << " Scenes Updated\n" << idx << "/" << num_scenes << " scenes in the list were recorded as new in the database.\n\n";
	    c.disconnect();
	    success = true;
	}
    }catch(const std::exception &e){logError(e.what());}
    return success;	
}

bool storeActorList(std::vector<Actor> &a, bool verbose){
    bool success = false;
    try{
		unsigned int updated = 0, added = 0, idx= 0;
		size_t num_actors = a.size();
		pqxx::connection c(DB_CREDS);
		if(!c.is_open()){	logError("Error opening actor database");}
		else 
		{
		    std::cout << "Connected to Database.\nAdding " << a.size() << " Actors." << std::endl;
		    for (std::vector<Actor>::iterator A = begin(a); A != end(a); ++A)
		    {
		    	++idx;
				if (A->size() > 1)	// don't even bother if there's nothing but a name
				{
				    std::string sql("SELECT FROM ACTORS WHERE (NAME LIKE " + sqlSafe(A->getName()) + ")");
				    pqxx::nontransaction N_search(c);
				    pqxx::result R(N_search.exec(sql.c_str()));
				    N_search.commit();
				    if (R.size() < 1)// if the actor wasn't found in the database, add them.
				    {
						++added;
						pqxx::nontransaction N_insert(c);
						N_insert.exec(A->sqlInsert().c_str());
						N_insert.commit();
						if (verbose)
							std::cout << addTabs(std::to_string(idx) + "/" + std::to_string(num_actors) + ":", 2) << "Added " << A->getName() << " to Database" << std::endl;					
				    }
				    else{
						try{
						    pqxx::result::const_iterator I = R.begin();
						    if (A->size() > ActorSize(I)){
								std::string updateSql(A->sqlUpdate());
								if (!updateSql.empty()){
									++updated;
								    pqxx::nontransaction N_update(c);
								    N_update.exec(updateSql.c_str());
								    N_update.commit();
								    if (verbose)
									    std::cout << addTabs(std::to_string(idx) + "/" + std::to_string(num_actors) + ":", 2) << "Updated the entry for " << A->getName() << std::endl;
								}
						    }
						}catch(std::exception &e){logError(e.what());}
				    }
				}
		    }
		    std::cout << std::endl << std::endl;
		    std::cout << "Added " << added << " new Actors." << std::endl;
		    std::cout << "Updated " << updated << " existing records." << std::endl;
		    std::cout << (added+updated) << "/" << num_actors << " Records from the List used to update the database." << std::endl << std::endl;
		    c.disconnect();
		    success = true;
		}
    }catch(const std::exception &e){logError(std::string(e.what()));}
    return success;
}

bool storeItem(std::string sql){
    bool success = false;
    try{
	pqxx::connection C(DB_CREDS);
	if (!C.is_open()){	logError("Error Opening Scene Database");	}
	else{
		pqxx::nontransaction N(C);
		N.exec(sql.c_str());
		N.commit();
	    C.disconnect();
	    success = true;
	    std::cout << "Successfully stored item in table\n";
	}
    }
    catch (std::exception e){	logError(std::string(e.what()));	}
    return success;	
}

bool storeScene(Scene &s){return storeItem(s.sqlInsert());}
bool storeActor(Actor &a){return storeItem(a.sqlInsert());}

//---------------------------------------------------------------------------------------------------------
//			RECORD CHECKING METHODS
//---------------------------------------------------------------------------------------------------------

std::vector<Scene> searchSceneTable(std::string searchterm){
	std::cout << "Searching Scene Database for the phrase \""  << searchterm << "\"" << std::endl;
	std::vector<Scene> list(0);
	try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){	logError("Error Opening Database Connection");}
		else{
			// Use case-insensitive sql regex to search for a term in the filename
			std::string sql("SELECT * FROM SCENES WHERE FILENAME ~* '*." + searchterm + "*.\';");	// need to finish this query definition
			pqxx::nontransaction N(c);
			pqxx::result R(N.exec(sql.c_str()));
			N.commit();
			for (pqxx::result::const_iterator I = R.begin(); I != R.end(); ++I)
			{
				list.push_back(sqlToScene(I));
			}
			std::cout << "Found " << list.size() << " Scenes matching the search criteria: \"" << searchterm << "\"" << std::endl;
			c.disconnect();
		}
	}catch(const std::exception &e){logError(std::string(e.what()));}
	return list;
}

bool isSceneStored(Filepath f){
    bool found = false;
    try{
	pqxx::connection C(DB_CREDS);
	if (!C.is_open())	{	logError("Can't Open Scene Database");	}
	else{
	    std::string sql("SELECT FROM SCENES WHERE (FILENAME = " + sqlSafe(f.filename()) + " AND FILEPATH = " + sqlSafe(f.filepath()) + ");");
	    pqxx::nontransaction N(C);
	    pqxx::result R(N.exec(sql));	// Execute the Query
	    N.commit();
	    // Check if anything is in the set of results
	    if (R.size() > 0)	{	found = true;	}
	    C.disconnect();
	}
    }catch(const std::exception &e){logError(std::string(e.what()));}
    return found;
}

bool isActorStored(std::string name){
    bool found = false;
    try{
	pqxx::connection C(DB_CREDS);
	if (!C.is_open()){logError("Can't Open Actor Database");}
	else{
	    std::string sql_search = "SELECT FROM ACTORS WHERE name = " + sqlSafe(name) + ";";
	    pqxx::nontransaction N_search(C);
	    pqxx::result R_search(N_search.exec(sql_search.c_str()));// Execute the Query
	    N_search.commit();
	    if (R_search.size() > 0)	{	found = true;	}
	    C.disconnect();
	}
    }catch(const std::exception &e){logError(std::string(e.what()));}
    return found;
}

bool isSceneStored(Scene &s){	return isSceneStored(Filepath(s.filepath(), s.filename()));	}
bool isActorStored(Actor &a){	return isActorStored(a.getName());	}

//-----------------------------------------------------------------------------------
//		RECORD RETREIVAL METHODS
//-----------------------------------------------------------------------------------
Scene loadScene(Filepath f){
	Scene s;
	try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){logError("Error Opening Database Connection.");}
		else
		{
				std::string sql_search("SELECT * FROM SCENES WHERE filename = "+sqlSafe(f.filename())+" AND filepath = "+sqlSafe(f.filepath())+";");
				pqxx::work N_search(c);
				pqxx::result R_search(N_search.exec(sql_search.c_str()));
				N_search.commit();
				if (!R_search.empty()){
					s = Scene(R_search.begin());

				}
				else{
					s = Scene(f);	
				}
		}
	}catch(std::out_of_range &e){
		std::string errStr = std::string(e.what()) + ":\t" + f.filename();
		logError(errStr);
	}
	return s;
}


Actor loadActor(std::string name){
	Actor n(name);
	try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){logError("Error Opening Database Connection.");}
		else
		{
			std::string sql_search("SELECT * FROM ACTORS where NAME = " + sqlSafe(name) + ";");
			pqxx::nontransaction N_search(c);
			pqxx::result R_search(N_search.exec(sql_search.c_str()));
			N_search.commit();
			if (R_search.size() > 0)
			{
			    pqxx::result::const_iterator I = R_search.begin();
			    n.clone(sqlToActor(I));
			}
			c.disconnect();
		}
	}catch(std::exception &e){logError(std::string(e.what()));}
	return n;
}

void loadScenes(std::vector<Scene> &list){
	std::cout << "Importing Scene List from Database\n";
	try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){logError("Error Opening Database Connection.");}
		else
		{
			omp_set_dynamic(0);
			pqxx::nontransaction N_collect(c);
			pqxx::result R_collect(N_collect.exec("SELECT * FROM SCENES"));
			N_collect.commit();
			#pragma omp parallel for num_threads(NUM_THREADS)
			for (size_t i = 0; i < R_collect.size(); ++i)
			{
				pqxx::result::const_iterator I = R_collect.begin() + i;	
				Scene s = sqlToScene(I);
				#pragma omp critical
				{
				    if (!SceneSearch(list, s.getFile()))    {list.push_back(s);}
				}
			}
			c.disconnect();
			std::cout << "Got " << list.size() << " Scenes from the Database." << std::endl;
			removeDuplicates(list);
		}
	}catch(std::exception &e){logError(std::string(e.what()));}
}
std::vector<Scene> loadScenesWhere(std::string sql_search)
{
	std::vector<Scene> sceneList(0);
	std::cout << "Importing Scene List from Database\n";
	try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){logError("Error Opening Database Connection.");}
		else
		{
			pqxx::nontransaction N_collect(c);
			pqxx::result R_collect(N_collect.exec(sql_search.c_str()));
			N_collect.commit();
			for (pqxx::result::const_iterator I = R_collect.begin(); I != R_collect.end(); ++I)
			{
				sceneList.push_back(sqlToScene(I));
			}
			std::cout << "Got " << sceneList.size() << " Scenes from the Database." << std::endl;
			c.disconnect();
			removeDuplicates(sceneList);
		}
	}catch(std::exception &e){logError(std::string(e.what()));}
	return sceneList;
}

void loadActors(std::vector<Actor> &list){
	std::cout << "Importing Actor List from Database\n";
	try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){logError("Error Opening Database Connection.");}
		else
		{
			pqxx::nontransaction N_collect(c);
			pqxx::result R_collect(N_collect.exec("SELECT * FROM ACTORS;"));
			N_collect.commit();
			size_t initialSize = list.size();
			for (pqxx::result::const_iterator I = R_collect.begin(); I != R_collect.end(); ++I)
			{	
				Actor a = sqlToActor(I);
				if (!inList(list, a.getName()))	// If we're adding to an existing list, make sure we don't add a second entry for the same actor.
					list.push_back(sqlToActor(I));
			}
			std::cout << "Got " << list.size() - initialSize << " actors from the Database." << std::endl;
			c.disconnect();
			removeDuplicates(list);
		}
	}catch(std::exception &e){logError(std::string(e.what()));}
}

void loadActorsScenes(std::vector<Scene> &list, std::string name){
	try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){	logError("Error Opening Database Connection");}
		else{
			// Use case-insensitive sql regex to search for a term in the filename
			std::string sql("SELECT * FROM " + SCENE_DB + " WHERE ACTOR1 = " + sqlSafe(name) + " OR actor2 = " + sqlSafe(name) + " OR actor3 = " + sqlSafe(name) + " OR actor4 = " + sqlSafe(name) + ";");	// need to finish this query definition
			pqxx::nontransaction N(c);
			pqxx::result R(N.exec(sql.c_str()));
			N.commit();
			for (pqxx::result::const_iterator I = R.begin(); I != R.end(); ++I)
			{
				list.push_back(sqlToScene(I));
			}
			c.disconnect();
		}
	}catch(const std::exception &e){logError(std::string(e.what()));}
}

std::vector<Scene> loadActorsScenes(std::string name){
	std::vector<Scene> list(0);
	loadActorsScenes(list, name);
	return list;
}

bool loadActorsFilmography(std::vector<Scene> &list, std::string name)
{
	bool loadedScenes = false;
	size_t initSize = list.size();
	try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){	logError("Error Opening Database Connection");}
		else{
			// Use case-insensitive sql regex to search for a term in the filename
			std::string sql("SELECT * FROM " + FILM_DB + " WHERE ACTOR = " + sqlSafe(name) + ";");
			pqxx::nontransaction N(c);
			pqxx::result R(N.exec(sql.c_str()));
			N.commit();
			for (pqxx::result::const_iterator I = R.begin(); I != R.end(); ++I)
			{
				list.push_back(sqlToScene(I));
			}
			c.disconnect();
		}
	}catch(const std::exception &e){logError(std::string(e.what()));}
	if (list.size() > initSize)
		loadedScenes = true;
	return loadedScenes;
}
//-----------------------------------------------------------------------------------
//		MISCELLANEOUS TOOLS
//-----------------------------------------------------------------------------------
std::vector<string_count_t> getCompanyCounts(void){
	std::vector<string_count_t> items(0);
	try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open())	{	logError("Error Opening Database Connection");}
		else{
			//std::string sql("SELECT DISTINCT COMPANY from SCENES order by COMPANY");
			pqxx::nontransaction N_count(c);
			pqxx::result R_count(N_count.exec("SELECT COMPANY, COUNT(*) as `num` FROM SCENES GROUP BY company;"));
			N_count.commit();
			for (pqxx::result::const_iterator I = R_count.begin(); I != R_count.end(); ++I)
			{
				items.push_back(string_count_t(I[0].as<std::string>(), I[1].as<int>()));
			}
			c.disconnect();
		}
	}catch(const std::exception &e){
		logError(std::string(e.what()));
	}
	return items;
}

//------------------------------------------------------------------------------
// CSV Functions
//------------------------------------------------------------------------------
void createActorCSV (std::vector<Actor> list, std::string filename)
{
	std::ofstream file;	
	file.open("actresses.csv", std::ios::out|std::ios::trunc);
	file << "name,aliases,birthday,city,country,measurements,hair,eyes,tattoos,piercings,pic" << std::endl;
	#pragma omp parallel for
    for (size_t i = 0; i < list.size(); i++)
	{
		// Get the most Complete set of data about the actor before adding them to the list
		list.at(i).updateBioFromWeb();
		// Get a std::string of the Actor's data formatted for the CSV, and output it to the file.
		std::string temp = list.at(i).getCSVLine();
		// Write and Flush the data to the file before continuing on.
		#pragma omp critical
		file << temp;

		file.flush();
	}
	file.close();
}

void createSceneCSV (std::vector<Scene> list, std::string filename)
{
	std::ofstream file;
	file.open(filename, std::ios::out|std::ios::trunc);
	file << "title,year,size,length,width,height,type,filepath,filename,series,company,rating,actress1,actress2,actress3,actress4,tags" << std::endl;
    for (size_t i = 0; i < list.size(); i++)
	{
		std::string dbLine = list.at(i).getCSVLine();
		std::cout << dbLine;
		file << dbLine;
		file.flush();
	}
	file.close();
}
/*
std::vector<Scene> sceneListFromCSV (std::string filename)
{
	std::ifstream file;
	std::vector<Scene> scenes;
	char buffer[1024];
	file.open(filename, std::ios::in);
	if (!file){
		logError("Couldn't open the Actor CSV File for reading");
		
	}
	else
	{
		int i = 0;
		for(std::string line; getline(file, line);)
		{
			std::vector<std::string>tokens = tokenizeString(line, std::string(","));

			if (tokens.size() < 17){
				logError("Error: CSV Line " << i << " doesn't have enough fields: \n" << std::string(buffer));
			}
			else{
				Scene s(tokens.at(0), tokens.at(1), tokens.at(2), tokens.at(3), tokens.at(4), tokens.at(5), tokens.at(6), tokens.at(7), tokens.at(8), tokens.at(9), tokens.at(10), tokens.at(11), tokens.at(12), tokens.at(13), tokens.at(14), tokens.at(15));
				std::vector<std::string> tags = tokenizeString(tokens.at(16), ", ");
				for(int i = 0; i < tags.size(); i++)
				{
					s.addTag(tags.at(i));
				}
				scenes.push_back(s);
			}
			i++;
		}
	}
	return scenes;
}
*/
