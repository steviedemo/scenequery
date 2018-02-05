#include "VideoPlayer.h"
#include <QString>
#include <QFileInfo>

VideoPlayer::VideoPlayer(QString file, QObject *parent):
    QObject(parent), filepath(file), command("/usr/local/bin/mplayer"), closed(false){
    this->process = new QProcess(this);
    connect(process, SIGNAL(started()), this, SLOT(videoStarted()));
    connect(process, SIGNAL(finished(int)), this, SLOT(videoClosed(int)));
}

void VideoPlayer::play(){
    process->setProgram(command);
    if (!QFileInfo(filepath).exists()){
        qWarning("Can't Play File - Not found on Device");
        emit error(QString("Video File %s could not be found").arg(filepath));
        emit videoStopped();
    } else {
        QStringList args;
        args << QString("%1").arg(filepath);
        process->setArguments(args);

        qDebug("Starting Video: %s", qPrintable(filepath));
        process->start();
        if (!process->waitForStarted()){
            qWarning("Error Starting Video");
            emit error(QString("Error Starting %1").arg(filepath));
            emit videoStopped();
        }
    }
}

void VideoPlayer::videoClosed(int){
    qDebug("Video Closed");
    this->closed = true;
    emit videoStopped();
}
void VideoPlayer::videoStarted(){
    qDebug("%s Started", qPrintable(filepath));
}
