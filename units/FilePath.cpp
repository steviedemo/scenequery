#include "FilePath.h"
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QRegularExpression>
#include <QTextStream>
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
    file.completeBaseName();
    file.fileName();

    this->extension = file.suffix();
}
FilePath::FilePath(QString fullPath){
    this->path="";
    this->name ="";
    this->extension = "";
    QRegularExpression rx("(\\/.+)\\/(.+)\\.(.+)");
    QRegularExpressionMatch m = rx.match(fullPath);
    if (m.hasMatch()){
        try{
            this->path = m.captured(1);
            this->name = m.captured(2);
            this->extension = m.captured(3);
        } catch(...){
            qCritical("Caught Exception Parsing FilePath Object out of string '%s'", qPrintable(fullPath));
        }
    }
}

FilePath::FilePath(const FilePath &f){
    this->name = f.name;
    this->path = f.path;
    this->extension = f.extension;
}
FilePath::~FilePath(){}

/** \brief Static Functions - Do not operate on class members. */
QString FilePath::currentPath   (void)          {   return QDir::currentPath();     }
QString FilePath::currentParent (void)          {   return QDir::currentPath().remove(QRegularExpression("/[\\w\\d\\s_]+[/]?"));    }
QString FilePath::getParent     (QString path)  {   return path.remove(QRegularExpression("/[\\w\\d\\s_]+[/]?"));   }

/** \brief Member Functions - Return data about the file object */
QString FilePath::getName       (void) const    {   return this->name;      }
QString FilePath::getPath       (void) const    {   return this->path;      }
QString FilePath::getExtension  (void) const    {   return this->extension; }
QString FilePath::parentPath    (void) const    {   return getParent(path);         }
bool    FilePath::isEmpty       (void) const    {   return (path.isEmpty() || name.isEmpty());  }
QString FilePath::absolutePath  (void) const    {   return QString("%1/%2.%3").arg(path).arg(name).arg(extension);  }

QString FilePath::sqlSafe() const {
    QString safe = this->absolutePath();
    if (safe.startsWith('\''))  {   safe = safe.remove(0, 1);               }
    if (safe.endsWith('\''))    {   safe = safe.remove(safe.size() - 1, 1); }
    return safe.remove(QRegularExpression("[\";]")).replace("'", "''");
}

QString FilePath::unixSafe() const {
    QString unix("");
    QString forbidden("()[]     ,&\\\'");
    QString temp = this->absolutePath();
    QTextStream out(&unix);
    foreach(QChar c, temp){
        if(forbidden.contains(c))
            out << '\'';
        out << c;
    }
    return unix;
}

bool FilePath::exists(void) const{
    bool fileExists = false;
    if (!path.isEmpty() && !path.isNull() && !name.isEmpty() && !name.isNull()){
        fileExists = QFileInfo(this->absolutePath()).exists();
    }
    return fileExists;
}

