#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
#include <QThread>
#include <QProcess>

class VideoPlayer : public QThread
{
    Q_OBJECT
public:
    VideoPlayer(QString filename, QObject *parent=0);
    void run();
private slots:
    void videoClosed(int);
    void videoStarted();
private:
    QObject *parent;
    QProcess *process;
    QString filepath, command;
    bool closed;
signals:
    void error(QString);
};

#endif // VIDEOPLAYER_H
