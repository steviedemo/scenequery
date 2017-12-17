#ifndef FILESCANNER_H
#define FILESCANNER_H
#include "FilePath.h"
#include "definitions.h"
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
#include <QMutex>

class FileScanner : public QThread
{
    Q_OBJECT
public:
    FileScanner(QString path="");
    ~FileScanner();
    bool setPath(QString);
    void run();
public slots:
    void receiveFileVector(QVector<FilePath>);
    void recursiveFileRead(QString);
private:
    QFileInfoList scan();
    QFileInfoList recursiveScan(QFileInfo());
    void initializeActors(void);
    void parseActorList(void);
    void parseSceneList(QFileInfoList);
    QString scanDir;
    QVector<FilePath> files;
    List<class Actor> actors;
    List<class Scene> scenes;
    int index;
    QMutex mx;
signals:
    void finished(List<class Scene>);
    void initProgress(int count);
    void updateProgress(int value);
    void closeProgress();
};

#endif // FILESCANNER_H
