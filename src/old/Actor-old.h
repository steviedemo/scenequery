#ifndef __ACTOR_H__
#define __ACTOR_H__
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include "Filepath.h"
#include "structs.h"
#include <QStandardItem>
#include <QString>
#include <QStringList>
#define ACTOR_SIZE	14
using namespace std;

bool byName(class Actor, class Actor);
bool byBirthday(class Actor, class Actor);
bool byScenes(class Actor, class Actor);

class Actor : public QStandardItem {
public:
	Actor(void);
	Actor(QString Name);
	Actor(QString Name, QString aliases, QString birthday, QString city,\
		  QString country, QString meas, QString hair, QString eyes,\
		  QString tatts, QString piercs, QString pic);
	Actor(const Actor &a);
	~Actor();
	
    // Setters
	void 						setName					(QString);
	void						setAliases				(QString);
	void						setBio					(bio_t);
	void						setHeight				(height_t);
	void						setHeight				(QString);
	void						setWeight				(int);
	void						setCity					(QString);
	void 						setNationality			(QString);
	void						setEthnicity			(QString);
	void						setMeasurements			(QString);
	void						setTattoos				(QString);
	void						setPiercings			(QString);
	void 						setStatus				(bool);
	void 						setHair					(QString);
	void						setEyes					(QString);
	void						setBirthday				(QString);
	void						setBirthday				(class Date);
	void						setCareerSpan			(int s, int f = -1);
	void						setHeadshot 			(Filepath h);
	void 						addScene				(class Scene scene);
	void						addScenes				(std::vector<class Scene>);
	void						addToFilmography		(class Scene scene);
	void						initFilmography			(std::vector<class Scene>);

	void						updateHeadshot			(void);
	bool						updateBioFromWeb		(void);
    bool						updateFromIAFD			(void);//(bool headshot = false);
	void						updateScenesWithAge		(void);

    // Getters
	void						clone					(Actor);
	QString 					getName					(void);
	QString						getAliases				(void);
	height_t					getHeight				(void);
	int							getWeight				(void);
	bio_t						getBio					(void);
	QString						getCity					(void);
	class Date					getBirthday				(void);
	QString						getBirthdayString		(void);
	QString						getNationality			(void);
	QString						getMeasurements			(void);
	QString						getHairColor			(void);
	QString						getEyeColor				(void);
	QString						getTattoos				(void);
	QString						getPiercings			(void);
	Filepath					getHeadshot				(void);
	bool						getStatus				(void);
	QList<QStandardItem>		getScenes				(void);
	int							getSceneCount			(void);
	QList<QStandardItem>		filmography				(void);
	size_t						filmographySize			(void);
	int							getCareerStart			(void);
	int							getCareerStop			(void);
    double						getDataUsage			(void);
// SQL
	QString						getCSVLine				(void);
    bool                        sqlInsert               (QString&, QStringList&);
//	string						sqlInsert				();
    bool                        sqlUpdate               (QString&, QStringList&);
//	string						sqlUpdate				();

// MISC
	bool						fetchFilmography		(void);
	QString 					toString				(void);
	void 						printProfile			(void);
	void						printScenes				(void);
	int							size					(void);
	double						completeness			(void);
// Checkers
	bool						isEmpty					(void);
	bool						hasBio					(void);
	bool						hasBirthday				(void);
	bool						hasHeadshot				(void);
	bool						hasFilmography			(void);
	void						addFilmography			(class Scene);
	void						addFilmography			(std::vector<class Scene>);
	bool 						hasScene				(class Scene);
private:
	int 						getActorNumber			(class Scene &);
	int 						getAgeInScene 			(class Scene &);
    void                        comma                   (QString &fields, QStringList &list, QString fieldname);
	// Members:
	QString name = 			DEFAULT_STRING;
	double dataUsage = 		DEFAULT_DOUBLE;
	class Date birthday;
	bool active;
	bio_t bio;
	QList<QStandardItem> localScenes, allScenes;
	Filepath headshot;
};

#endif
