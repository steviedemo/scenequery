#include "FileScanner.h"
#include "Scene.h"
#include "SceneList.h"
#include "Actor.h"
#include "SceneParser.h"
#include <qtconcurrentrun.h>
#include <QtConcurrent>
#include <QFutureSynchronizer>
#include <QMediaMetaData>
#include <QThreadPool>
#include <QVideoFrame>
#include <QDir>
#include <unistd.h>
FileScanner::FileScanner(const QString &path):
    index(0){
    this->rootFolders << path;
    this->actorList = {};
    this->sceneList = {};
    this->setTerminationEnabled(true);
}
FileScanner::FileScanner(const QStringList &paths):
    rootFolders(paths), index(0){
    this->actorList = {};
    this->sceneList = {};
    this->setTerminationEnabled(true);
}

FileScanner::~FileScanner(){}

/** \brief Slot to break out of the thread's Event Loop */
void FileScanner::stopThread(){
    qDebug("File Thread Stopping...");
    this->keepRunning = false;
}

void FileScanner::db_to_fs_receiveUnsavedScenes(QFileInfoList newFiles){
    this->scanFiles = newFiles;
    qDebug("File Scanner Got list of %d files not in database to create", newFiles.size());
}

void MiniScanner::run(){
    if (!root.isEmpty() && QDir(root).exists()){
        qDebug("Scanning root folder %s...", qPrintable(root));
        this->infoList = FileScanner::recursiveScan(root);
    }
    emit done(infoList);
}
void FileScanner::miniscanComplete(QFileInfoList list){
    QMutexLocker ml(&fileListMx);
    foreach(QFileInfo i, list){
        if (!foundFiles.contains(i)){
            this->foundFiles << i;
        }
    }
    ++threadsComplete;
}

void FileScanner::getFiles(){
    if (!rootFolders.isEmpty()){
        this->threadPool = new QThreadPool();
        foreach(QString folder, rootFolders){
            MiniScanner *m = new MiniScanner(folder);
            connect(m, SIGNAL(done(QFileInfoList)), this, SLOT(miniscanComplete(QFileInfoList)));
            this->threadPool->globalInstance()->start(m);
            this->threadsStarted++;
        }
        threadPool->waitForDone(180000);
        delete threadPool;
    }
}

/** \brief Thread's Main Event Loop */
void FileScanner::run(){
    if (rootFolders.isEmpty()){
        qWarning("Can't Scan Empty Directory. File Scanner Stopping.");
        return;
    }
    this->keepRunning = true;
    this->index = 0;
    this->sceneList = {};
    foreach(QString folder, rootFolders){
        QFileInfoList currList = recursiveScan(folder);
        foreach(QFileInfo item, currList){
            this->foundFiles << item;
        }
    }
    if (this->foundFiles.size() > 0){
        emit fs_to_db_checkScenes(foundFiles);
        if (scanFiles.size() > 0){
            // Run multiple threads that create scene objects
            sceneList = makeScenes(scanFiles);
            // Store the scenes to the database
            emit fs_to_db_storeScenes(sceneList);
            // Extract a list of unique names from the scenes.
            nameList = getNames(sceneList);
            // Send the list to the database which will check which actors are already stored there,
            // and pass on a list of those that arent to the curl thread to build the actor profiles.
            qDebug("File Scanner Passing List of Names to SQL Thread to check if they are already in the database");
            emit fs_to_db_checkNames(nameList);
        } else {
            emit showError("No New Files");
        }
    } else {
        emit showError("No files to scan in!");
    }
    qDebug("File Scanner Thread Finished");
}

/** \brief Scan in all files within a directory, and recursively scan each sub-directory.
 *  \param QFileInfo rootFolder:    The folder to scan for files.
 *  \return QFileInfoList:          List of files in this folder and in all subdirectories.
 */
QFileInfoList FileScanner::recursiveScan(const QFileInfo &rootFolder){
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
    index = 0;
    emit startProgress(QString("Scanning in %1 files").arg(fileList.size()), fileList.size());
    QFutureSynchronizer<void> sync;
    foreach(QFileInfo f, fileList){
        sync.addFuture(QtConcurrent::run(this, &FileScanner::parseScene, f));
    }
    sync.waitForFinished();
    emit closeProgress(QString("File Scanning Complete!"));
    return sceneList;
}

/** \brief Scan a single scene in from a QFileInfo object into a Scene Object, and add it to the vector of sceneList.
 *  \param QFileInfo file:  File to parse
 */
void FileScanner::parseScene(QFileInfo f){
    subMx.lock();
    QString file = f.absoluteFilePath();
    subMx.unlock();
    SceneParser parser(file);
    parser.parse();
    ScenePtr scene = QSharedPointer<Scene>(new Scene(parser));
    subMx.lock();
    emit updateProgress(index++);
    sceneList.push_back(scene);
    subMx.unlock();
}

/** \brief Accept a list of Scenes and make a list of names of the actors in each scene.
 *  \param SceneList sceneList scanned in
 *  \return QStringList actor names.
 */
QStringList FileScanner::getNames(SceneList list){
    QStringList names;
    int index = 0;
    emit startProgress("Scanning in actor names", list.size());
    foreach(ScenePtr s, list){
        QString name = s->getActor(0);
        if (!name.isEmpty()){
            if (!names.contains(name)){
                qDebug("Adding '%s' to list of names to add to the database", qPrintable(name));
                names << name;
            }
        }
        if (++index % 50 == 0){
            emit updateProgress(index);
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
    this->nameList.clear();
    ActorList actorList = {};
    QStringList oldNames;
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
                if (!oldNames.contains(name) && !nameList.contains(name)){
                    nameList << name;
                    ++added;
                }
            }
            emit updateProgress(++index);
        }
        emit closeProgress(QString("Found %1 new actor names. Updating Actors..."));
        // If there are new names, then create actor members for them.
        if (nameList.size() > 0){
            foreach(QString name, nameList){
                actorList.push_back(QSharedPointer<Actor>(new Actor(name)));
            }
            qDebug("Emitting Request to Curl Thread for Bio Updates");
            emit updateBios(actorList);
        }
    }
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
