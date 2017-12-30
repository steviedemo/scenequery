#include "FileScanner.h"
#include "Scene.h"
#include "SceneList.h"
#include "Actor.h"
#include "sceneParser.h"
#include <qtconcurrentrun.h>
#include <QtConcurrent>
#include <QFutureSynchronizer>
#include <QDir>

FileScanner::FileScanner(QString path):
    scanDir(path), index(0){
    this->actors = QVector<QSharedPointer<Actor>>();
    this->setTerminationEnabled(true);
}
FileScanner::~FileScanner(){

}

void FileScanner::run(){
    this->keepRunning = true;
    qDebug("Scanner Thread Started.");
    while (this->keepRunning){
        sleep(1);
    }
    qDebug("Scanner Thread Stopped");
}


void FileScanner::scanFolder(QString rootPath){
    this->index = 0;
    SceneList sceneList;
    QStringList names;
    qDebug("Scanning '%s' for files...", qPrintable(rootPath));
    QFileInfoList fileList;
    if (QDir(rootPath).exists()){
        emit updateProgress(0);
        emit updateStatus(QString("Scanning %1...").arg(rootPath));
        fileList = recursiveScan(rootPath);
        if (fileList.size() > 0){
            sceneList = makeScenes(fileList);
            names = getNames(sceneList);
        } else {
            showError("No files to scan in!");
        }
    } else {
        emit showError(QString("%1 doesn't exist!").arg(rootPath));
    }
    emit scanComplete(sceneList, names);
}

/** \brief Scan in all files within a directory, and recursively scan each sub-directory.
 *  \param QFileInfo rootFolder:    The folder to scan for files.
 *  \return QFileInfoList:          List of files in this folder and in all subdirectories.
 */
QFileInfoList FileScanner::recursiveScan(QFileInfo rootFolder){
    QStringList nameFilters;
    QFileInfoList fileList;
    if (!rootFolder.exists()){
        return fileList; // Stop here if the folder isn't found
    }
    nameFilters << "*.wmv" << "*.mp4" << "*.flv" << "*.mpg" << "*.mpeg" << "*.wma";     // Set the filename filters.
    fileList.append(QDir(rootFolder.absoluteFilePath()).entryInfoList(nameFilters, QDir::Files | QDir::NoDotAndDotDot)); // Get Files from this directory
    qDebug("Got %d Files from '%s'", fileList.size(), qPrintable(rootFolder.absoluteFilePath()));
    // Get List of Sub Directories
    QFileInfoList subFolders = QDir(rootFolder.absoluteFilePath()).entryInfoList(QStringList(), QDir::Dirs|QDir::NoDotAndDotDot);
    foreach(QFileInfo subDir, subFolders){
        fileList.append(recursiveScan(subDir));
    }
    return fileList;
}

/** \brief Accept a list of QFileInfo object and make each into a scene object.
 *  \param QFileInfoList fileList:  List of files that were scanned in by the recursive scan process
 *  \return SceneList list:         List of Scenes created by parsing the passed files.
 */
SceneList FileScanner::makeScenes(QFileInfoList fileList){
    emit startProgress(QString("Scanning in %1 files").arg(fileList.size()), fileList.size());
    QFutureSynchronizer<void> sync;
    foreach(QFileInfo f, fileList){
        sync.addFuture(QtConcurrent::run(this, &FileScanner::parseScene, f));
    }
    sync.waitForFinished();
    emit closeProgress(QString("File Scanning Complete!"));
    return scenes;
}

/** \brief Scan a single scene in from a QFileInfo object into a Scene Object, and add it to the vector of scenes.
 *  \param QFileInfo file:  File to parse
 */
void FileScanner::parseScene(QFileInfo f){
    mx.lock();
    FilePath file(f.absolutePath(), f.completeBaseName(), f.suffix());
    mx.unlock();
    sceneParser parser(file);
    parser.parse();
    ScenePtr scene = QSharedPointer<Scene>(new Scene(parser));
    mx.lock();
    emit updateProgress(index++);
    scenes.push_back(scene);
    mx.unlock();
}

/** \brief Accept a list of Scenes and make a list of names of the actors in each scene.
 *  \param SceneList scenes scanned in
 *  \return QStringList actor names.
 */
QStringList FileScanner::getNames(SceneList list){
    QStringList names;
    int total = list.size();
    emit startProgress("Scanning in actor names", list.size());
    for (int i = 0; i < total; ++i){
        ScenePtr s = list.at(i);
        emit updateProgress(i);
        QStringList cast = s->getActors();
        for (int j = 0; j < cast.size(); ++j){
            if (!names.contains(cast.at(j))){
                qDebug("Adding '%s' to list of names", qPrintable(cast.at(j)));
                names.append(cast.at(j));
            }
        }
    }
    emit closeProgress("Name list completed.");
    qDebug("Got %d names from scanned scenes", names.size());
    return names;
}

void FileScanner::scanForActors(SceneList list, ActorList actors){
    // Reset values to 0;
    this->index = 0;
    this->added = 0;
    this->newNames.clear();
    ActorList actorList = {};
    QStringList oldNames, newNames;
    // Start processing on list
    if (list.size() > 0){
        // Get a list of names already in the actor list.
        foreach(ActorPtr a, actors){
            if (!oldNames.contains(a->getName())){
                oldNames << a->getName();
            }
        }
        startProgress(QString("Scanning Scenes for new Actor Names"), list.size());
        // Scan list of scenes for new actor names.
        emit startProgress(QString("Scanning %1 Scenes for actors").arg(list.size()), list.size());
        foreach(ScenePtr s, list){
            foreach(QString name, s->getActors()){
                if (!oldNames.contains(name) && !newNames.contains(name)){
                    newNames << name;
                    ++added;
                }
            }
            emit updateProgress(++index);
        }
        emit closeProgress(QString("Found %1 new actor names. Updating Actors..."));
        // If there are new names, then create actor members for them.
        if (newNames.size() > 0){
            foreach(QString name, newNames){
                actorList.push_back(QSharedPointer<Actor>(new Actor(name)));
            }
            qDebug("Emitting Request to Curl Thread for Bio Updates");
            emit updateBios(actorList);
        }
    }
}

void FileScanner::receiveUpdatedActors(ActorList list){
    this->actors = list;
    emit scanComplete(actors);
}

void FileScanner::receiveFileVector(QVector<FilePath> fileVector){
    this->files = fileVector;
}

void FileScanner::stopThread(){
    qDebug("File Thread Stopping...");
    this->keepRunning = false;
}


ActorList FileScanner::parseActorList(SceneList sceneList){
    ActorList actorList;
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


//----------------------------------------------------------------
//		THUMBNAILS
//----------------------------------------------------------------

/** \brief Generate Thumbnails for the video file that the Scene passed refers to. */
#warning Incomplete function for thumbnail generation.
bool FileScanner::generateThumbnail(ScenePtr s){
    if (s->exists()){
        /*
        QString cmd = QString("ffmpg -i %1 -vf fps=%2 scale=\'min(%3, iw):-1\' %4").arg(file.unixSafe()).arg()
        std::string command("ffmpeg -i " + file.unixSafe() + " -vf fps=" + THUMBNAIL_RATE + " scale=\'min(" + THUMBNAIL_MAX_SIZE + "\\, iw):-1\' " + destination);
        shell_it(command);
        */
    }
    return false;
}
