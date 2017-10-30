#ifndef FILESCANNER_H
#define FILESCANNER_H
#include "FilePath.h"
// Qt Stuff
#include <QObject>
#include <QStandardItem>
#include <QString>
#include <QThread>
// Data Parsing
#include <QDate>
#include <QFile>
#include <QFileInfo>
// Collection Structures
#include <QVector>
#include <QMap>

class FileScanner
{
    Q_OBJECT
public:
    FileScanner();
    FileScanner(QString path);
    ~FileManager();
    bool setPath(QString);
    bool scan(QVector<class Actor> &a, QVector<class Scene> &s);

public slots:
    void receiveFileVector(QVector<FilePath>);
    void recursiveFileRead(QString);
private:
    void parseActorList(void);
    void parseSceneList(void);
    QString scanDir;
    QVector<FilePath> files;
    QVector<class Actor> actors;
    QVector<class Scene> scenes;

};

#endif // FILESCANNER_H
