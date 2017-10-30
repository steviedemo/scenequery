#ifndef __TOOLS_H__
#define __TOOLS_H__
#include "Filepath.h"
#include <QString>
#include <QList>
#include <QStandardItem>
void		 					fetchIAFDBios			(QList<QStandardItem> &actors, bool useList = true);
void							fetchBios 				(QList<QStandardItem> &actors, bool useList = true);
void						    updateBios				(QList<QStandardItem> &actors, QList<QStandardItem> &scenes);
void						    updateAgesInScenes		(QList<QStandardItem> &actors, QList<QStandardItem> &scenes);
void updateWeights(void);

void						    toTextFile				(std::vector<Filepath> &, QString, bool);
std::string						newFilename				(class Scene &);
bool	    					commitName				(class Scene &, QString newName);
bool	    					commitName				(class Scene &);
QList<QStandardItem> 			topScenes				(QList<QStandardItem> &scenes, int);
void							updateReleaseDates		(void);
void							hardResetBios			(void);
void 							softResetBios			(void);

#endif
