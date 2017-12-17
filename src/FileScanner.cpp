#include "FileScanner.h"
#include "Scene.h"
#include "Actor.h"
#include <qtconcurrentrun.h>
#include <QtConcurrent>
#include <QFutureSynchronizer>
#include <QDir>
FileScanner::FileScanner(QString path):
    scanDir(path), index(0){
}

void FileScanner::run(){
    if (!scanDir.isEmpty()){
        qDebug("Scanning Directory '%s'", qPrintable(scanDir));
        QFileInfoList infoList = scan();
        if (infoList.size() > 0){
            this->index = 0;
            emit initProgress(infoList.size());
            qDebug("Scanning in %d scenes", infoList.size());
            QFutureSynchronizer<void> sync;
            foreach(QFileInfo f, infoList){
                FilePath file(f.absolutePath(), f.completeBaseName());
                sync.addFuture(QtConcurrent::run(this, &FileScanner::addScene, file));
            }
            sync.waitForFinished();
        }
    } else {
        qWarning("Unable to scan for files - no path provided");
    }
    emit finished(sceneList);
}

void FileScanner::addScene(FilePath file){
    Scene scene(file);
    mx.lock();
    emit updateProgress(index++);
    scenes.push_back(scene);
    mx.unlock();
}

List<Actor> FileScanner::parseActorList(List<Scene>sceneList){
    List<Actor> actorList;
    // For every scene in the list
    foreach(ScenePtr s, sceneList){
        QStringList cast = s->getActors();
        // Look at all actors in this scene
        foreach(QString name, cast){
            ActorPtr a = QSharedPointer<Actor>(new Actor(name));
            if (!actorList.contains(a)){
                actorList.push_back(a);
            }
        }
    }
    return actorList;
}

void FileScanner::initializeActors(List<Actor> actorList){
    foreach(ActorPtr a, actorList){
        a->updateBio();
    }
}

List<Scene> FileScanner::parseSceneList(QFileInfoList infoList){
    // Make a Scene object for every item in the list of files.
    foreach(QFileInfo info, infoList){
        FilePath f(info.absolutePath(), info.completeBaseName());
        QSharedPointer<Scene> scene = QSharedPointer<Scene>(new Scene(f));
        scenes.push_back(scene);
    }
}

QFileInfoList FileScanner::scan(){
    QDir root = QDir(scanDir);
    QFileInfoList files;
    if (!root.exists()){
        qCritical("Filepath '%s' Does not exist.", qPrintable(scanDir));
        return fileList;
    }
    QStringList folderNameFilters;
    QFileInfoList subFolders = root.entryInfoList(folderNameFilters, QDir::Dirs|QDir::NoDotAndDotDot);
    // Recursively Scan Subdirectories and create a List of FilePath Objects.
    foreach(QFileInfo subdir, subFolders){
        files.append(recursiveScan(subdir));
    }
    return files;
}

QFileInfoList FileScanner::recursiveScan(QFileInfo rootFolder){
    QStringList nameFilters;
    QFileInfoList fileList();
    // Stop here if the folder isn't found
    if (!rootFolder.exists()){
        return fileList;
    }
    // Get Files from this directory
    fileList << QDir(rootFolder.absoluteFilePath()).entryInfoList(nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    qDebug("Got %d Files from '%s'", fileList.size(), qPrintable(rootFolder.absoluteFilePath()));

    // Get List of Sub Directories
    QFileInfoList subFolders = QDir(rootFolder.absoluteFilePath()).entryInfoList(nameFilters, QDir::Dirs|QDir::NoDotAndDotDot);
    foreach(QFileInfo subDir, subFolders){
        fileList.append(recursiveScan(subDir));
    }
    return fileList;
}
