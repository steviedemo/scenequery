#-------------------------------------------------
#
# Project created by QtCreator 2017-07-31T11:14:28
#
#-------------------------------------------------

QT       += core gui sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      = SceneQuery
TEMPLATE    = app
CONFIG      += debug

MOC_DIR     = build
OBJECTS_DIR = build
UI_DIR      = build

QMAKE_CXXFLAGS =        -std=c++0x
QMAKE_CFLAGS_DEBUG =    -std=gnu99
QMAKE_CFLAGS_RELEASE =  -std=gnu99
QMAKE_LFLAGS += -F/System/Library/Frameworks -L/usr/lib
# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    /usr/local/include \
    src \
    units \


LIBS += \
    -lcurl \
    -L/usr/local/lib \
    -framework DiskArbitration \
    -framework IOKit \
    -framework OpenGL \
    -framework AGL

SOURCES += src/main.cpp\
    src/mainwindow.cpp \
    src/config.cpp \
    src/curlTool.cpp \
    src/genericfunctions.cpp \
    src/qsqldbhelper.cpp \
    src/sceneParser.cpp \
    src/sql.cpp \
    src/FileScanner.cpp \
    units/Actor.cpp \
    units/Biography.cpp \
    units/FilePath.cpp \
    units/Height.cpp \
    units/Rating.cpp \
    units/Scene.cpp \
    qtcurl/QtCUrl.cpp \
    src/DatabaseThread.cpp \
    src/ActorThread.cpp

HEADERS  += src/mainwindow.h \
    units/FilePath.h \
    units/Rating.h \
    units/Scene.h \
    units/Actor.h \
    units/Biography.h \
    units/Height.h \
    src/qsqldbhelper.h \
    src/genericfunctions.h \
    src/sceneParser.h \
    qtcurl/QtCUrl.h \
    src/sql.h \
    src/curlTool.h \
    src/config.h \
    src/FileScanner.h \
    src/definitions.h \
    src/DatabaseThread.h \
    src/ActorThread.h

FORMS    += forms/mainwindow.ui

QMAKE_CLEAN += \
    build/* \
    QSceneQuery.pro.user \
    Makefile
