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
    QString getName(void);
    QString getPath(void);
    QString absolutePath(void);
    QString getExtension(void);
    static QString currentPath();
    static QString parentPath();
    static QString getParent(QString path);

    bool exists();
private:
    QString name;
    QString path;
    QString extension;
};

#endif // FILEPATH_H
