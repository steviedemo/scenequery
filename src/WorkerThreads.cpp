#include "WorkerThreads.h"
#include "FilePath.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QVector>

void FileReaderThread::run(){
    bool retry_status = true;
    QString folder("");
    while(retry_status && (folder.isEmpty() || folder.isNull())){
        folder = selectFolder();
        if (folder.isEmpty() || folder.isNull()){
            retryDialog("Error Reading in File.", "Error Reading in File. Retry?");
        }
    }

    if (!folder.isEmpty() && folder.isNull()){
        QVector<FilePath> files = recursiveRead(folder);
        emit sendFiles(files);
    }
}

QVector<FilePath> FileReaderThread::selectFolder(void){
    QString folder(""), returnValue("");
    folder = QFileDialog::getOpenFileName(this, "Select a folder to read in", "/");
    if (QDir(folder).exists()){
        returnValue = folder;
    }
    return returnValue;
}

bool FileReaderThread::retryDialog(QString title, QString text){
    bool status = false;
    QMessageBox m;
    m.setIcon(QMessageBox::Warning);
    m.setWindowTitle(title);
    m.setText(text);
    m.addButton(QMessageBox::Retry);
    m.addButton(QMessageBox::Cancel);
    if (m.exec() == QMessageBox::Retry){
        status = true;
    }
    return status;
}

QVector<FilePath> FileReaderThread::recursiveRead(QString rootFolder){
    QVector<FilePath> newFiles;
    QDir folder(rootFolder);
    if (folder.exists()){

        // Get List of Files from this folder.
        QStringList files = folder.entryList(QStringList() << "*.mp4" << "*.mpg" << "*.flv" << "*.avi" << "*.wmv", QDir::Files);
        QStringListIterator i(files);
        while(i.hasNext()){ newFiles.push_back(FilePath(rootFolder, i.next())); }

        // Get List of sub-folders.
        QStringList subFolders = folder.entryList(QStringList() << "", QDir::Dirs);

        // Get List of Files from each Subfolder.
        QStringListIterator f(subFolders);
        while(f.hasNext()){
            QVectorIterator<FilePath> sf(readIn(QString("%1/%2").arg(rootFolder).arg(f.next())));
            while(sf.hasNext()){
                newFiles.push_back(sf.next());
            }
        }
    }
    return newFiles;
}
