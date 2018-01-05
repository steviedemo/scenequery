#ifndef FILEPATH_H
#define FILEPATH_H
#include <QString>
class FilePath
{

public:
    FilePath();
    FilePath(QString absolute_path);
    FilePath(QString path, QString name);
    FilePath(QString path, QString name, QString extensien);
    FilePath(const FilePath &other);
    ~FilePath();
    void setFile        (QString);
    void setFile        (const FilePath &f);
    QString getName     (void) const;
    QString getPath     (void) const;
    QString getExtension(void) const;
    QString absolutePath(void) const;
    QString parentPath  (void) const;
    QString unixSafe    (void) const;
    QString sqlSafe     (void) const;
    bool exists         (void) const;
    bool isEmpty        (void) const;
    int     size        (void) const;

    static QString currentPath();           // App's current path
    static QString currentParent();         // App's parent Path
    static QString getParent(QString path); // Return the folder one level above the path passed
private:
    QString name;
    QString path;
    QString extension;
    QString absolute;
};

#endif // FILEPATH_H
