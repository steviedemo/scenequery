#include "FilePath.h"
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QRegularExpression>
#include <QVector>
FilePath::FilePath(){
    this->name="";
    this->path="";
}
FilePath::FilePath(QString path, QString name, QString extension){
    this->path = path;
    this->name = name;
    this->extension = extension;
}
FilePath::FilePath(QString path, QString name){
    QFileInfo file(QString("%1/%2").arg(path).arg(name));
    this->path = path;
    this->name = file.baseName();
    this->extension = file.completeSuffix();
}
FilePath::FilePath(QString fullPath){
    QFileInfo file(fullPath);
    this->path = file.canonicalPath();
    this->name = file.baseName();
    this->extension = file.completeSuffix();
}

FilePath::FilePath(const FilePath &f){
    this->name = f.name;
    this->path = f.path;
    this->extension = f.extension;
}
FilePath::~FilePath(){}

QString FilePath::getName()      const {   return this->name;      }
QString FilePath::getPath()      const {   return this->path;      }
QString FilePath::getExtension() const {   return this->extension; }
QString FilePath::absolutePath() const {
    return QString("%1/%2.%3").arg(path).arg(name).arg(extension);
}

QString FilePath::currentPath(void){
    return QDir::currentPath();
}
QString FilePath::parentPath(){
    return QDir::currentPath().remove(QRegularExpression("/[\\w\\d\\s_]+[/]?"));
}
QString FilePath::getParent(QString path){
    return path.remove(QRegularExpression("/[\\w\\d\\s_]+[/]?"));
}
QString FilePath::unixSafe(){
    QVector<QChar> unsafe = { '(', ')', '[', ']', ' ', ',', '&', '&', '\''};
    QString unix("");
    QString absolute = absolutePath();
    for (int i = 0; i < absolute.size(); ++i){
        QChar c = absolute.at(i);
        if (unsafe.contains(c)){
            unix.push_back('\\');
        }
        unix.push_back(c);
    }
    return unix;
}

bool FilePath::exists(void){
    bool fileExists = false;
    if (!path.isEmpty() && !path.isNull() && !name.isEmpty() && !name.isNull()){
        fileExists = QFileInfo(this->absolutePath()).exists();
    }
    return fileExists;
}

