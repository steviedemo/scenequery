#ifndef WORKERTHREADS_H
#define WORKERTHREADS_H
#include <QThread>
#include <QVector>

class FileReaderThread : public QThread
{
public:
    FileReaderThread();
    ~FileReaderThread();
    void run();
private:
    QVector<class FilePath> selectFolder(void);
    QVector<class FilePath> recursiveRead(QString);
    bool retryDialog(QString title, QString text);
signals:
    void sendFiles(QVector<class FilePath>);
};

#endif // WORKERTHREADS_H
