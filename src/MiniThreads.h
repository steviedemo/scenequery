#ifndef MINITHREADS_H
#define MINITHREADS_H
#include <QThread>
#include <QRunnable>
#include "definitions.h"
class FileRenamer : public QThread
{
    Q_OBJECT
public:
    explicit FileRenamer(ScenePtr s, QString newName, QObject *parent = 0):
        QThread(parent), newName(newName), scene(s){}
    void run();
private:
    QString newName;
    ScenePtr scene;
signals:
    void saveToDatabase(ScenePtr);
    void error(QString);
};

#endif // MINITHREADS_H
