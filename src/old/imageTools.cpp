#include <omp.h>
#include <vector>
#include <string>
#include <fstream>
#include <boost/regex.hpp>
#include "database.h"
#include "curlRequests.h"
#include "database.h"
#include "Scene.h"
#include "Actor-old.h"
#include "Filepath.h"
#include "stringTools.h"
#include "imageTools.h"
#include "output.h"

#define BUFFSIZE 1028
#define NUM_THREADS 32
#define THUMBNAIL_RATE "1/300" // one every 5 minutes
#define THUMBNAIL_MAX_SIZE "100"
std::string shell_it(std::string cmd)
{
	std::string c = cmd.c_str();
    std::string output = "";
    char buffer[BUFFSIZE];
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try{
        while (!feof(pipe)){
            if (fgets(buffer, BUFFSIZE, pipe)  != NULL)
                output += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return output;	
}

//----------------------------------------------------------------
//		HEADSHOTS
//----------------------------------------------------------------
bool headshotExists(Actor &a)
{
	Filepath headshot(HEADSHOT_PATH, toImage(a.getName()), "jpg");
	return headshot.exists();
}
bool headshotExists(std::string name)
{
	Filepath headshot(HEADSHOT_PATH, toImage(name, "jpg"));
	return headshot.exists();
}


void getHeadshotLocation(std::string name, Filepath &f)
{
	f.setPath(std::string(HEADSHOT_PATH));
	f.setName(toImage(name, "jpg"));
}

bool 	wget	(std::string link, Filepath destination)
{
	bool success = false;
	std::string command("wget -O " + destination.fullpath() + " " + link);
	shell_it(command.c_str());
	if (destination.exists())
		success = true;
	else
		std::cout << " Failed to Download link " << link << std::endl;
	return success;
}

std::string getPhotoLink(std::string name)
{
	std::string line("");
	std::string html = (getIAFDhtml(getIAFDURL(name)));
	if (!html.empty())
	{
		try{
			std::vector<std::string> lines = tokenizeString(html, '\n');
			for (size_t i = 0; i < lines.size() && line.empty(); ++ i)
			{
				if (contains(lines.at(i), "<div id=\"headshot\">")){
					line = lines.at(i);
				}
			}
			removeStr(line, "src=\"", true);
			removeStr(line, "\"></div>", false);
		}catch(std::exception &e){ logError(e.what()); }
	}
	if (line.empty())
	{
		std::cout << "Link to Headshot Image not Found" << std::endl;
	}
	return line;
}

Filepath downloadHeadshot(std::string name)
{
	Filepath imgLocation = new Filepath();
	Filepath destination = imgLocation;
	getHeadshotLocation(name, imgLocation);
	std::string link = getPhotoLink(name);
	//std::cout  << "Name:\t" << name << "\nLink:\t" << link << "\nDest.:\t" << imgLocation.fullpath() << std::endl << std::endl;
	if (wget(link, imgLocation)){
		//std::cout << "Image Downloaded to " << imgLocation.fullpath() << std::endl;
		destination = imgLocation;
	}
	else{
		destination = Filepath();
	}
	return destination;
}
Filepath downloadHeadshot(Actor &a)
{
	Filepath destination = new Filepath();
	getHeadshotLocation(a.getName(), destination);
	if (headshotExists(a))
	{
		return destination;
	}
	std::string link = getPhotoLink(a.getName());
	if (wget(link, destination))
		return destination;
	else
		return new Filepath();
}
void downloadHeadshots(std::vector<Actor> &a)
{
	size_t idx = 0;
	omp_set_dynamic(0);
	#pragma omp parallel for num_threads(NUM_THREADS)
	for (size_t i = 0; i < a.size(); ++i)
	{
		if (a.at(i).hasBio())
		{
			Filepath f("", "");
			std::string link = getPhotoLink(a.at(i).getName());
			getHeadshotLocation(a.at(i).getName(), f);
			
			if (!f.empty() && !link.empty())
				wget(link, f);
			#pragma omp critical
			{
				std::cout << ++idx << "/" << a.size() << ": " << a.at(i).getName();
				
				std::cout << std::endl;
			}
		}
	}
}
/*
void storeImage(Filepath &f, int table_id)
{
	try{
		pqxx::connection c(DB_CREDS);
		if (!c.is_open()){	logError("Error Opening Database Connection");}
		else{
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
}
*/
// TODO: Add OID conversion code.
/*
void storeHeadshot(Filepath &f, std::string name)
{
	std::string sql("INSERT INTO HEADSHOTS (filename, name) VALUES (");
	sql += f.filename() + ", " + name + ");";
	storeItem(sql);
}
*/
//----------------------------------------------------------------
//		THUMBNAILS
//----------------------------------------------------------------
std::string getThumbnailFormat(std::string filename)
{
	return std::string(THUMBNAIL_PATH + toImageFormatString(filename));
}
bool thumbnailExists(Scene &s)
{
	Filepath thumbnail(THUMBNAIL_PATH, toImage(s.filename(), "png"));
	return thumbnail.exists();
}

void generateThumbnails(Scene &s)
{
	if (s.exists())
	{
		std::string destination = getThumbnailFormat(s.filename());
		std::string command("ffmpeg -i " + s.unix() + " -vf fps=" + THUMBNAIL_RATE + " scale=\'min(" + THUMBNAIL_MAX_SIZE + "\\, iw):-1\' " + destination);
		shell_it(command);
	}
}
void generateThumbnails(std::vector<Scene> &s)
{
	size_t idx = 0;
	omp_set_dynamic(0);
	#pragma omp parallel for num_threads(NUM_THREADS)
	for (size_t i = 0; i < s.size(); ++i)
	{
		#pragma omp critical
		{
			std::cout << ++idx << "/" << s.size() << ":\t" << s.at(i).filename() << std::endl;
		}
		generateThumbnails(s.at(i));
	}
}
