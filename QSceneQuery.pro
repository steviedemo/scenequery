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

QMAKE_MACOSX_DEPLOYMENT_TARGET=10.9#-mmacosx-version-min
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
    -I/usr/local/opt/qt/bin
PKGCONFIG  = /usr/local/opt/qt/lib/pkgconfig
LIBS += \
    -lcurl \
    -L/usr/local/lib \
    -framework DiskArbitration \
    -framework IOKit \
    -framework OpenGL \
    -framework AGL  \
    -L/usr/local/opt/qt/lib \
    -L/System/Library/Frameworks/ImageIO.framework/Versions/A/Resources

SOURCES += \
    src/ActorThread.cpp \
    src/config.cpp \
    src/curlTool.cpp \
    src/DatabaseThread.cpp \
    src/FileScanner.cpp \
    src/genericfunctions.cpp \
    src/main.cpp\
    src/mainwindow.cpp \
    src/qsqldbhelper.cpp \
    src/sceneParser.cpp \
    src/sql.cpp \
    units/Actor.cpp \
    units/Biography.cpp \
    units/FilePath.cpp \
    units/Height.cpp \
    units/Rating.cpp \
    units/Scene.cpp \
    qtcurl/QtCUrl.cpp \
    src/sqlconnection.cpp \
    query.cpp

HEADERS  += \
    src/ActorThread.h \
    src/config.h \
    src/curlTool.h \
    src/DatabaseThread.h \
    src/definitions.h \
    src/FileScanner.h \
    src/genericfunctions.h \
    src/mainwindow.h \
    src/qsqldbhelper.h \
    src/sceneParser.h \
    src/sql.h \
    units/Actor.h \
    units/Biography.h \
    units/FilePath.h \
    units/Height.h \
    units/Rating.h \
    units/Scene.h \
    qtcurl/QtCUrl.h \
    src/sqlconnection.h \
    query.h

FORMS    += forms/mainwindow.ui

QMAKE_CLEAN += \
    build/* \
    QSceneQuery.pro.user \
    Makefile

STATECHARTS += \
    forms/GUI_State_chart.scxml

DISTFILES += \
    scripts/collect_exif.sh
