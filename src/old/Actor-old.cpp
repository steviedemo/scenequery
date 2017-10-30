#include <algorithm>
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdexcept>
#include <sys/types.h>
#include <vector>
#include "Actor-old.h"
#include "Date.h"
#include "Filepath.h"   
#include "Scene.h"
#include "curlRequests.h"
#include "imageTools.h"
#include "stringTools.h"
#include "structs.h"
#include "output.h"

#include "qsqldbhelper.h"

#define SEPARATOR	"\";\""


extern Date todays_date;
bool byName(Actor a, Actor b)
{
    if (a.getName() < b.getName())
    return true;
    else
    return false;
}
//------------------------------------------------------------------------------------------------------------------
//											CONSTRUCTORS
//------------------------------------------------------------------------------------------------------------------
Actor::Actor(void)
{
	name = "";
	localScenes.resize(0);
}
Actor::Actor(std::string Name)
{
	name = Name;
	localScenes.resize(0);
}
// Copy Constructor
Actor::Actor(const Actor &a)
{
    name = a.name;
    localScenes = a.localScenes;
    headshot = a.headshot;
    bio = a.bio;
}
void Actor::clone(Actor a)
{
    name = a.getName();
    localScenes = a.getScenes();
    headshot = a.getHeadshot();
    bio = a.getBio();
}

// Destructor
Actor::~Actor(){	localScenes.resize(0);	}

bool SceneFind(std::vector<Scene> &list, Filepath f)
{
	bool found = false;
	for (std::vector<Scene>::iterator I = begin(list); I != end(list) && !found; ++I)
	{
		if (I->fullpath() == f.fullpath())
		{
			found = true;
		}
	}
	return found;
}
bool SceneFind(std::vector<Scene> &list, Scene &scene)
{
	return SceneFind(list, scene.getFile());
}

//------------------------------------------------------------------------------------------------------------------
//											DATA SETTERS
//------------------------------------------------------------------------------------------------------------------

// Setters
void 	Actor::setName			(std::string n)		{ 	name = n;					}
void 	Actor::setBio			(bio_t b)			{	bio = b;					}
void	Actor::setAliases		(std::string s)		{	bio.aliases = s;			}
void 	Actor::setHair			(std::string h)		{	bio.hairColor = h;			}
void 	Actor::setEyes			(std::string e)		{	bio.eyeColor = e;			}
void 	Actor::setStatus		(bool status)		{	bio.active = status;		}
void	Actor::setCity			(std::string c)		{	bio.city = c;				}
void 	Actor::setNationality	(std::string n)		{	bio.nationality = n;		}
void 	Actor::setEthnicity		(std::string e)		{	bio.ethnicity = e;			}
void 	Actor::setMeasurements	(std::string m)		{	bio.measurements = m;		}
void 	Actor::setWeight		(int w)				{ 	bio.weight = w;				}
void 	Actor::setHeight		(height_t h)		{	bio.height = h;				}
void	Actor::setHeight		(std::string h)		{ 	bio.height = stringToHeight(h);			}
void	Actor::setPiercings		(std::string p)		{	bio.piercings = p;			}
void 	Actor::setTattoos		(std::string t)		{	bio.tattoos = t;			}
void	Actor::setBirthday		(Date d)			{	bio.birthdate = d;			}
void 	Actor::setBirthday		(std::string d)		{	bio.birthdate = textDateToStruct(d);	}
void 	Actor::setHeadshot 		(Filepath h)		{ 	headshot = h;	}
//void	Actor::updateHeadshot	(void)				{	headshot = fetchHeadshot(name);		}

void	Actor::initFilmography	(std::vector<Scene> s){	allScenes = s;				}
void 	Actor::addToFilmography	(Scene s)			{	allScenes.push_back(s);		}

void 	Actor::setCareerSpan	(int s, int f)		{	bio.start = s; bio.end = f;	}
void 	Actor::addScene			(Scene scene)		
{	

	if (localScenes.size() > 0){
		if (!SceneFind(localScenes, scene)){
			dataUsage += (double)scene.getSize();
			localScenes.push_back(scene);
		}
	}
	else{
		dataUsage += (double)scene.getSize();
		localScenes.push_back(scene);
	}
}
void 	Actor::addScenes		(std::vector<Scene> list)
{	
	for (std::vector<Scene>::iterator I = begin(list); I != end(list); ++I)
	{
		addScene(*I);
	}
}



//------------------------------------------------------------------------------------------------------------------
//											DATA RETRIEVAL
//------------------------------------------------------------------------------------------------------------------

// Getters
std::string 		Actor::getName			(void)	{	return name;			}
bio_t				Actor::getBio			(void)	{	return bio; }
std::string			Actor::getAliases		(void)	{	return bio.aliases;			}
int					Actor::getWeight		(void)	{ 	return bio.weight;			}
height_t			Actor::getHeight		(void)	{ 	return bio.height;			}
std::string			Actor::getCity			(void)	{	return bio.city;	}
Date				Actor::getBirthday		(void)	{	return bio.birthdate;		}
std::string			Actor::getNationality	(void)	{	return bio.nationality;		}
std::string			Actor::getMeasurements	(void)	{	return bio.measurements;	}
std::string			Actor::getHairColor		(void)	{	return bio.hairColor;		}
std::string			Actor::getEyeColor		(void)	{	return bio.eyeColor;		}
std::string			Actor::getTattoos		(void)	{	return bio.tattoos;			}
std::string			Actor::getPiercings		(void)	{	return bio.piercings;		}
Filepath			Actor::getHeadshot		(void)	{	return headshot;			}
bool				Actor::getStatus		(void)	{	return bio.active;			}
std::vector<Scene>	Actor::getScenes		(void)	{	return localScenes;		}
std::vector<Scene>	Actor::filmography		(void)	{	return allScenes;		}
size_t				Actor::filmographySize 	(void)	{	return allScenes.size();	}
int					Actor::getCareerStart	(void)	{	return bio.start;		}
int					Actor::getCareerStop	(void)	{	return bio.end;		}
int					Actor::getSceneCount	(void)	{	return localScenes.size();	}
double 				Actor::getDataUsage		(void)	{	return dataUsage/1024.0;}

// Return Total space taken up by all localScenes, in Gigabytes.
int Actor::getActorNumber(Scene &s){
	int actorNumber = -1;
    size_t a = 0;
	for(a = 0; a < s.getCastSize(); a++)
	{
		if (name == s.getActorNumber(a))
		{
			actorNumber = a;
			a = s.getCastSize();
		}
	}
	return a;
}
int Actor::getAgeInScene(Scene &s)
{
	int num = getActorNumber(s);
	return s.getAgeOfActorNum(num);
}

//------------------------------------------------------------------------------------------------------------------
//											CHECKING FOR PRESENCE OF DATA
//------------------------------------------------------------------------------------------------------------------
// Checkers:
bool 				Actor::empty			(void)	{	return name.empty();			}

bool Actor::hasHeadshot	(void)	
{	
	bool exists = false;
	if (headshotExists(name))
	{
		headshot = Filepath(HEADSHOT_PATH, toImage(name));
		exists = true;
	}
	return exists;
}
bool Actor::hasBio			(void)	
{
	if (bio.empty())
		return false;
	else
		return true;
}

bool Actor::hasScene (Scene s)
{
	bool notFound = true;
	for (std::vector<Scene>::iterator i = localScenes.begin(); i != localScenes.end() && notFound; ++i)
	{
		if ((s.filename() == i->filename()) && (s.filepath() == i->filepath()))
			notFound = false;
	}
	return notFound;
}
bool Actor::hasBirthday(void)
{
	if (bio.birthdate.y() > 0 && bio.birthdate.m() > 0 && bio.birthdate.d() > 0)
		return true;
	else 
		return false;
}

//------------------------------------------------------------------------------------------------------------------
//											UPDATING
//------------------------------------------------------------------------------------------------------------------
bool 	Actor::hasFilmography	(void)
{
	if (allScenes.size() > 0)
		return true;
	else
		return false;
}
bool	Actor::fetchFilmography	(void)
{
	bool found = false;
	std::string html = getIAFDURL(name);
	if (!html.empty())
	{
		found = true;
		allScenes = getOnlineFilmography(html);
		// Add the actor and the age in the scene to the info about the scene.
		for (size_t i = 0; i < allScenes.size(); i++){
			allScenes.at(i).addActor(name, bio.birthdate);
		}
	}
	return found;
}
void 	Actor::addFilmography(Scene s)
{
	bool found = false;
	if (allScenes.size() > 0)
	{
		for (std::vector<Scene>::iterator S = allScenes.begin(); S != allScenes.end() && !found; ++S)
		{
			if ((s.getTitle() == S->getTitle()) && (s.getCompany() == S->getCompany()) && (s.getReleaseDate().on(S->getReleaseDate())))
			{
				found = true;
			}
		}
	}
	if (!found)
		allScenes.push_back(s);
}
void Actor::addFilmography(std::vector<Scene> s)
{
	for (std::vector<Scene>::iterator S = begin(s); S != end(s); ++S)
	{
		addFilmography(*S);
	}
}

bool 	Actor::updateFromIAFD 	()//(bool getHeadshot)
{
	bool found = false;
	std::string html = getIAFDURL(name);
	if (!html.empty())
	{
		found = true;
		allScenes = getOnlineFilmography(html);
		// Add the actor and the age in the scene to the info about the scene.
		for (size_t i = 0; i < allScenes.size(); i++){
			allScenes.at(i).addActor(name, bio.birthdate);
		}
	//	if (getHeadshot)
			//headshot = getBioPicFromHTML(html, toImage());
	}
	return found;
}

bool	Actor::updateBioFromWeb	(void)
{
	bool found = false;
	if(fetchFreeonesBio(fetchHTML(getFreeonesURL(name)), bio)){
	    updateScenesWithAge();
	    found = true;
	}
	else{
	    log("Couldn't Find Bio Data on the Web for " + name);
	}
	return found;
}

// set the age of the actor in this particular scene.
void Actor::updateScenesWithAge(){
	if (hasBirthday())	// no processing if the birthdate is missing
	{
		try{			// Catch any Out-Of-Range errors.
			for (auto s = localScenes.begin(); s != localScenes.end(); ++s)
			{
				if (s->hasReleaseDate())
				{
                    int billingNo = -1;
					int ageIn = age(bio.birthdate, s->getReleaseDate());
					for (size_t i = 0; i < s->getCastSize() && (billingNo == -1); ++i){
						if (name == s->getActorNumber(i)) {
                            billingNo = (int)i;
						}
					}
					if (billingNo > -1){
						s->setAgeOfActorNum(ageIn, billingNo);
					}
				}
			}
		}catch(std::out_of_range &e){
			std::cerr << __FUNCTION__ << " @ " << __LINE__ << " in " << __FILE__ <<  ": " << e.what() << std::endl; 
		}
	}
}

//------------------------------------------------------------------------------------------------------------------
//											PRINTING
//------------------------------------------------------------------------------------------------------------------

std::string Actor::toString(void)
{
	int ratedScenes = 0;
	double totalSize = 0.0;
	size_t num_scenes = localScenes.size();
	std::string o("\n------------------------------------------------------------------------------------------------------------------------------------------------\n\n"+name+"\n");
	if (bio.birthdate.y() > 0 && todays_date.y() > 0){
	    int currAge = age(bio.birthdate, todays_date);
	    if (currAge > -1)	{	o+= "\tCurrent Age: " + std::to_string(currAge) + "\n";	}
	}
	o += bio.toString();
	o += "\n" + std::to_string(num_scenes) + " Scenes:\n";
	int i  = 1;
	for (std::vector<Scene>::iterator S = begin(localScenes); S != end(localScenes); ++S)
	{
		int height = S->getHeight();
		int width = S->getWidth();
		int ageIn = age(bio.birthdate, S->getReleaseDate());
		int sceneSize = (int)S->getSize();
		double sceneLength = S->getLength();
		std::string picSize = std::to_string(height) + " x " + std::to_string(width);
		o += "\n\t#" + std::to_string(i) + "\t" + S->getTitle() + "\n";
		if (ageIn > 0)					{	o += "\t\tAge in Scene:\t" 	+ std::to_string(ageIn) + "\n";					}
		if (width > -1 && height > -1)	{	o += "\t\tPicture Size: " 	+ picSize + " pixels\n";						}
		if (sceneLength > 0)			{	o += "\t\tLength:\t\t" 		+ std::to_string(sceneLength) + " minutes.\n";	}
		if (sceneSize > 0)				{	o += "\t\tSize:\t\t" 		+ std::to_string(sceneSize) + " MB\n";			}
		if (S->rated())					{	o += "\t\tRating:\t"		+ S->getRating() + "\n";	++ratedScenes;		}
		o += "\t\t" + S->fullpath() + "\n";
		totalSize += sceneSize;
		++i;
	}
	double ratedRatio = 100.0*(double)ratedScenes/(double)localScenes.size();
	o += "\n\t" + name + " accounts for a total of " + std::to_string(totalSize/1024.0) + " GB of disk space.\n";
	o += "\t" + std::to_string(ratedScenes) + " out of " + std::to_string(localScenes.size()) + ", or " + std::to_string((int)ratedRatio) + "% are rated.\n\n";
	return o;
}

void Actor::printScenes(void){
	std::cout << toString() << std::endl;
}

std::string	Actor::getCSVLine()
{
	std::string line = "\"" + name + SEPARATOR + bio.aliases + SEPARATOR + bio.birthdate.sql() + SEPARATOR;
	line += bio.city + SEPARATOR + bio.nationality + SEPARATOR + bio.measurements + SEPARATOR;
	line += bio.hairColor + SEPARATOR + bio.eyeColor + ";\"" + bio.tattoos + SEPARATOR + bio.piercings + SEPARATOR + headshot.fullpath() + "\"\n";
	return line;
}

//------------------------------------------------------------------------------------------------------------------
//											SPECIAL PURPOSE
//------------------------------------------------------------------------------------------------------------------

// returns number between 0 and 14 (inclusive) based on how many data fields are filled out
int Actor::size(void)
{
	int count = bio.count();					// 0-12
	if (!name.empty())		{	++count;	}	// 13
	if (hasHeadshot())		{	++count;	}	// 14
	return count;	
}

// 0 - 1
double Actor::completeness(void)
{
    double ratio = (double)size();
    ratio /= ACTOR_SIZE;
    return ratio;
}

/*
std::string Actor::sqlInsert(void)
{
	std::string fields = "INSERT INTO ACTORS (NAME";
	std::string values = ") VALUES ("+ sqlSafe(getName());
	if (!bio.aliases.empty())		{	fields += ",ALIASES";	values += ","+sqlSafe(bio.aliases);}
	if (isDateValid(birthday))		{	fields += ",BIRTHDAY";	values += ","+sqlSafe(bio.birthdate);}
	if (!bio.city.empty())			{	fields += ",CITY";		values += ","+sqlSafe(bio.city);}
	if (!bio.nationality.empty())	{	fields += ",COUNTRY";	values += ","+sqlSafe(bio.nationality);}
	if (!bio.ethnicity.empty())		{	fields += ",ETHNICITY";	values += ","+sqlSafe(bio.ethnicity);}
	if (bio.height.nonzero())		{	fields += ",HEIGHT";	values += "," + sqlSafe(bio.height.toCm());}
	if (bio.weight != 0)			{	fields += ",WEIGHT";	values += "," + sqlSafe(bio.weight);}
	if (!bio.measurements.empty())	{	fields += ",MEASUREMENTS";	values += ","+sqlSafe(bio.measurements);}
	if (!bio.hairColor.empty())		{	fields += ",HAIR";		values += ","+sqlSafe(bio.hairColor);}
	if (!bio.eyeColor.empty())		{	fields += ",EYES";		values += ","+sqlSafe(bio.eyeColor);}
	if (!bio.tattoos.empty())		{	fields += ",TATTOOS";	values += ","+sqlSafe(bio.tattoos);}
	if (!bio.piercings.empty())		{	fields += ",PIERCINGS";	values += ","+sqlSafe(bio.piercings);}
	if (hasHeadshot())				{	fields += ",PHOTO";		values += ","+sqlSafe(toImage(name));}
	return fields + values + ");";	
}
*/
bool Actor::sqlInsert(QString& query, QStringList& list)
{
    list.clear();
    QString fields("INSERT INTO ACTORS (NAME");
    QString values("?");
    list << sqlSafe(getName());
    if (!bio.aliases.empty())       {   comma(fields, list, "ALIASES");     list << sqlSafe(bio.aliases);       values.append(", ?"); }
    if (isDateValid(birthday))      {   comma(fields, list, "BIRTHDAY");    list << sqlSafe(birthday);          values.append(", ?"); }
    if (!bio.city.empty())          {   comma(fields, list, "CITY");        list << sqlSafe(bio.city);          values.append(", ?"); }
    if (!bio.nationality.empty())   {   comma(fields, list, "COUNTRY");     list << sqlSafe(bio.nationality);   values.append(", ?"); }
    if (!bio.ethnicity.empty())     {   comma(fields, list, "ETHNICITY");   list << sqlSafe(bio.ethnicity);     values.append(", ?"); }
    if (bio.height.nonzero())       {   comma(fields, list, "HEIGHT");      list << sqlSafe(bio.height);        values.append(", ?"); }
    if (bio.weight != 0)            {   comma(fields, list, "WEIGHT");      list << sqlSafe(bio.weight);        values.append(", ?"); }
    if (bio.measurements.empty())   {   comma(fields, list, "MEASUREMENTS");list << sqlSafe(bio.measurements);  values.append(", ?"); }
    if (!bio.hairColor.empty())     {   comma(fields, list, "HAIR");        list << sqlSafe(bio.hairColor);     values.append(", ?"); }
    if (!bio.eyeColor.empty())      {   comma(fields, list, "EYES");        list << sqlSafe(bio.eyeColor);      values.append(", ?"); }
    if (!bio.tattoos.empty())       {   comma(fields, list, "TATTOOS");     list << sqlSafe(bio.tattoos);       values.append(", ?"); }
    if (!bio.piercings.empty())     {   comma(fields, list, "PIERCINGS");   list << sqlSafe(bio.piercings);     values.append(", ?"); }
    if (hasHeadshot())              {   comma(fields, list, "PHOTO");       list << sqlSafe(toImage(name));     values.append(", ?"); }
    query = QString("%1) VALUES (%2);").arg(fields).arg(values);
    return (list.size() > 0);
}


void Actor::comma(QString &fields, QStringList &list, QString fieldname)
{
    if (!list.isEmpty()){
        fields.append(",");
    }
    fields.append(QString(" %1 = ?").arg(fieldname));
}


bool Actor::sqlUpdate(QString& query, QStringList& list)
{
    bool success = false;
    QString fields("UPDATE actors SET ");
    list.clear();
    if (!bio.aliases.empty())       {   fields.append("ALIASES = ?");       list << sqlSafe(bio.aliases);       }
    if (isDateValid(birthday))      {   comma(fields, list, "BIRTHDAY");    list << sqlSafe(birthday);          }
    if (!bio.city.empty())          {   comma(fields, list, "CITY");        list << sqlSafe(bio.city);          }
    if (!bio.nationality.empty())   {   comma(fields, list, "COUNTRY");     list << sqlSafe(bio.nationality);   }
    if (!bio.ethnicity.empty())     {   comma(fields, list, "ETHNICITY");   list << sqlSafe(bio.ethnicity);     }
    if (bio.height.nonzero())       {   comma(fields, list, "HEIGHT");      list << sqlSafe(bio.height);        }
    if (bio.weight != 0)            {   comma(fields, list, "WEIGHT");      list << sqlSafe(bio.weight);        }
    if (bio.measurements.empty())   {   comma(fields, list, "MEASUREMENTS");list << sqlSafe(bio.measurements);  }
    if (!bio.hairColor.empty())     {   comma(fields, list, "HAIR");        list << sqlSafe(bio.hairColor);     }
    if (!bio.eyeColor.empty())      {   comma(fields, list, "EYES");        list << sqlSafe(bio.eyeColor);      }
    if (!bio.tattoos.empty())       {   comma(fields, list, "TATTOOS");     list << sqlSafe(bio.tattoos);       }
    if (!bio.piercings.empty())     {   comma(fields, list, "PIERCINGS");   list << sqlSafe(bio.piercings);     }
    if (hasHeadshot())              {   comma(fields, list, "PHOTO");       list << sqlSafe(toImage(name));     }
    if (list.size() > 0){
        fields.append(" WHERE NAME = ?;");
        list << sqlSafe(name);
        success = true;
    }
    return success;
}
/*
std::string Actor::sqlUpdate(void)
{
	bool prev = false;
	std::string fields("UPDATE actors SET ");

	if (!bio.aliases.empty())
	{	
		prev = true;
		fields += "ALIASES = "+sqlSafe(bio.aliases);
	}
	if (isDateValid(birthday))
	{	
		if (prev)	fields.push_back(',');
		else 		prev = true;
		fields += "BIRTHDAY = "+sqlSafe(bio.birthdate);
	}
	if (!bio.city.empty())
	{	
		if (prev)	fields.push_back(',');
		else 		prev = true;
		fields += "CITY = "+sqlSafe(bio.city);
	}
	if (!bio.nationality.empty())
	{	
		if (prev)	fields.push_back(',');
		else 		prev = true;
		fields += "COUNTRY = "+sqlSafe(bio.nationality);
	}
	if (!bio.ethnicity.empty())
	{	
		if (prev)	fields.push_back(',');
		else 		prev = true;
		fields += "ETHNICITY = "+sqlSafe(bio.ethnicity);
	}
	if (bio.height.nonzero())
	{	
		if (prev)	fields.push_back(',');
		else 		prev = true;
		fields += "HEIGHT = " + sqlSafe(bio.height.toCm());
	}
	if (bio.weight != 0)
	{
		if (prev)	fields.push_back(',');
		else 		prev = true;
		fields += "WEIGHT = " + sqlSafe(bio.weight);
	}
	if (!bio.measurements.empty())
	{	
		if (prev)	fields.push_back(',');
		else		prev = true;
		fields += "MEASUREMENTS = "+sqlSafe(bio.measurements);
	}
	if (!bio.hairColor.empty())
	{	
		if (prev)	fields.push_back(',');
		else 		prev = true;
		fields += "HAIR = "+sqlSafe(bio.hairColor);
	}
	if (!bio.eyeColor.empty())
	{	
		if (prev)	fields.push_back(',');
		else 		prev = true;
		fields += "EYES = "+sqlSafe(bio.eyeColor);
	}
	if (!bio.tattoos.empty())
	{	
		if (prev)	{fields.push_back(',');}
		else 		{prev = true;}
		fields += "TATTOOS = "+sqlSafe(bio.tattoos);
	}
	if (!bio.piercings.empty())
	{	
		if (prev)	{fields.push_back(',');}
		else 		{prev = true;}
		fields += "PIERCINGS = "+sqlSafe(bio.piercings);
	}
	if (hasHeadshot())
	{	
		if (prev)	{fields.push_back(',');}
		else 		{prev = true;}
		fields += "PHOTO = " + sqlSafe(toImage(name));
	}
	if (prev)
		fields += " WHERE NAME = " + sqlSafe(name) + ";";	
	else
		fields = "";
	return fields;
}

*/
