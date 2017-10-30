#include "FilePath.h"
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QRegularExpression>
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

QString FilePath::getName() { return this->name; }
QString FilePath::getPath() { return this->path; }
QString FilePath::absolutePath(){
    return QString("%1/%2.%3").arg(name).arg(path).arg(extension);
}
QString FilePath::getExtension(){
    return this->extension;
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
bool FilePath::exists(void){
    bool fileExists = false;
    if (!path.isEmpty() && !path.isNull() && !name.isEmpty() && !name.inNull()){
        fileExists = QFileInfo(this->absolutePath()).exists();
    }
    return fileExists;
}

