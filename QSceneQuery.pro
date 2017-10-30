#-------------------------------------------------
#
# Project created by QtCreator 2017-07-31T11:14:28
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      = sq
TEMPLATE    = app
CONFIG      += debug
DESTDIR     = bin
QMAKE_CXXFLAGS += -std=c++0x
MOC_DIR     = build
OBJECTS_DIR = build
UI_DIR      = build

QMAKE_CXXFLAGS = -std=c++0x -fopenmp
QMAKE_CFLAGS_DEBUG = -std=gnu99
QMAKE_CFLAGS_RELEASE = -std=gnu99

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


#_BOOST_PATH = /usr/local/Cellar/boost/1.62.0
#_PQXX_PATH  = /usr/local/Cellar/libpqxx/4.0.1_2
#_CELLAR     = /usr/local/Cellar
INCLUDEPATH += /usr/local/include \
            /usr/local/opt/llvm/include \
            src \
            units
          #  $${_CELLAR}/libiomp/20150701/include/libiomp

LIBS += \ #-L$${_CELLAR}/libiomp/20150701/lib \
        -L/usr/local/lib

LIBS += -lpqxx -lpq -lcurl -lgomp -lcurses -lncurses -lreadline #-lboost_regex

SOURCES += src/main.cpp\
    src/mainwindow.cpp \
    src/config.cpp \
    src/curlTool.cpp \
    src/genericfunctions.cpp \
    src/qsqldbhelper.cpp \
    src/sceneParser.cpp \
    src/sql.cpp \
    src/WorkerThreads.cpp \
    units/Actor.cpp \
    units/FilePath.cpp \
    units/Rating.cpp \
    units/Scene.cpp \
    src/old/Actor-old.cpp \
    src/old/collectors.cpp \
    src/old/curlRequests.cpp \
    src/old/database.cpp \
    src/old/Date.cpp \
    src/old/imageTools.cpp \
    src/old/interface.cpp \
    src/old/lists.cpp \
    src/old/main-old.cpp \
    src/old/output.cpp \
    src/old/stringTools.cpp \
    src/old/structs.cpp \
    src/old/systemInterface.cpp \
    src/old/tests.cpp \
    src/old/tools.cpp \
    qtcurl/QtCUrl.cpp \
    src/FileScanner.cpp \
    units/Height.cpp

HEADERS  += src/mainwindow.h \
    src/old/collectors.h \
    src/old/curlRequests.h \
    src/old/database.h \
    src/old/Date.h \
    units/FilePath.h \
    units/Rating.h \
    units/Scene.h \
    units/Actor.h \
    src/old/imageTools.h \
    src/old/interface.h \
    src/old/lists.h \
    src/old/output.h \
    src/old/stringTools.h \
    src/old/structs.h \
    src/old/systemInterface.h \
    src/old/tests.h \
    src/old/tools.h \
    src/qsqldbhelper.h \
    src/WorkerThreads.h \
    src/genericfunctions.h \
    src/sceneParser.h \
    qtcurl/QtCUrl.h \
    src/old/Actor-old.h \
    src/sql.h \
    src/curlTool.h \
    src/config.h \
    src/FileScanner.h \
    units/Biography.h \
    units/Height.h

FORMS    += forms/mainwindow.ui

QMAKE_CLEAN += \
    bin/*
    build/*
    QSceneQuery.pro.user
