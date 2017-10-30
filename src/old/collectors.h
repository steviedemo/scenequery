#ifndef __COLLECTORS_H__
#define __COLLECTORS_H__
#include <QList>
#include <QStandardItem>
#include <QString>
void						toUpdate    					(QList<QStandardItem> &, bool, int cutoffQuality = 720, int cutoffSize = 200, bool ratedOnly = false);
void					    removeScenesWithoutAddedDate	(QList<QStandardItem> &);
void					    removeScenesWithoutReleaseDate	(QList<QStandardItem> &);
void						totalSceneList_byCompany(QString);
QList<QStandardItem>    	getScenesAddedOn		(class Date);
QList<QStandardItem>    	getScenesAddedBefore	(class Date);
QList<QStandardItem>    	getScenesAddedAfter		(class Date);
QList<QStandardItem>    	getScenesReleasedOn		(class Date);
QList<QStandardItem>    	getScenesReleasedBefore	(class Date);
QList<QStandardItem>    	getScenesReleasedAfter	(class Date);
void					    listScenesAddedOn		(class Date, QString);
void					    listScenesAddedBefore	(class Date, QString);
void				      	listScenesAddedAfter	(class Date, QString);
void						listScenesReleasedOn	(class Date, QString);
void	 					listScenesReleasedBefore(class Date, QString);
void 						listScenesReleasedAfter	(class Date, QString);


#endif
