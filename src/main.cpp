#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <QMutex>
QMutex mx;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg){
    mx.lock();
    const char * localMsg = qPrintable(msg);
    if (type == QtDebugMsg || type == QtInfoMsg){
        std::cout << localMsg << std::endl;
    } else if (type == QtWarningMsg || type == QtCriticalMsg){
        std::cerr << ((type == QtWarningMsg) ? "Warning: " : "Critical: ") << localMsg;
        std::cerr << " (" << context.file << "::" << context.function << "::" << context.line << ")" << std::endl;
    } else {
        std::cerr << "Fatal Error: " << localMsg << " (" << context.file << "::" << context.function << "::" << context.line << ")" << std::endl;
    }
    mx.unlock();
}


int main(int argc, char *argv[]){
    qInstallMessageHandler(messageHandler);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
