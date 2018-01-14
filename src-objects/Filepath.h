#ifndef FILEPATH_H
#define FILEPATH_H
#include <QPair>
#include <QString>

class Filepath: QPair<QString,QString>
{
public:
    Filepath();
    void setName(QString);
    void setPath(QString);
    void setAbsolutePath(QString);
    QString getPath();
    QString getName();
    QString getAbsolutePath();
    QPair<QString,QString> getPair();

private:
    QPair<QString,QString> file;
};

#endif // FILEPATH_H
