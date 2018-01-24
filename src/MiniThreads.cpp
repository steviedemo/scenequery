#include "MiniThreads.h"
#include <QFileInfo>
#include <QFile>
#include <QPair>
#include "Scene.h"
void FileRenamer::run(){
    if (!scene.isNull() && !newName.isEmpty()){
        QPair<QString,QString> fileInfo = scene->getFile();
        QString fullpath = scene->getFullpath();
        QString newPath = QString("%1/%2").arg(scene->getFolder()).arg(newName);
        QString oldName = scene->getFilename();
        QPair<QString,QString> newFileInfo;
        newFileInfo.first = fileInfo.first;
        newFileInfo.second = newName;
        scene->setFile(newFileInfo);
        QFile file(fullpath);
        bool saved = false;
        if (fullpath == newPath){
            saved = true;
        } else {
            saved = file.rename(fullpath, newPath);
        }
        if (!saved){
            emit error(QString("Error Renaming\n%1\n---->\n%2").arg(oldName).arg(newName));
        } else {
            emit saveToDatabase(scene);
            emit done(scene);
        }

   } else {
        qWarning("Error: File Renamer passed a null scene pointer");
    }
}
