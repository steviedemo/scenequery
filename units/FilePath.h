#ifndef FILEPATH_H
#define FILEPATH_H
#include <QString>
class FilePath
{

public:
    FilePath();
    FilePath(QString);
    FilePath(QString, QString);
    FilePath(QString, QString, QString);
    FilePath(const FilePath&);
    ~FilePath();
    QString getName(void) const;
    QString getPath(void) const;
    QString absolutePath(void) const;
    QString getExtension(void) const;
    static QString currentPath();
    static QString parentPath();
    static QString getParent(QString path);
    QString unixSafe();
    QString sqlSafe();
    bool exists();
private:
    QString name;
    QString path;
    QString extension;
};

#endif // FILEPATH_H
