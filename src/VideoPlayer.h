#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
#include <QThread>
#include <QProcess>

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    VideoPlayer(QString filename, QObject *parent=0);
public slots:
    void play();
private slots:
    void videoClosed(int);
    void videoStarted();
private:
    QObject *parent;
    QProcess *process;
    QString filepath, command;
    bool closed;
signals:
    void videoStopped();
    void error(QString);
};

#endif // VIDEOPLAYER_H
