#-------------------------------------------------
#
# Project created by QtCreator 2017-07-31T11:14:28
#
#-------------------------------------------------

QT       += core gui sql concurrent network multimedia widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      = SceneQuery
TEMPLATE    = app
CONFIG      += debug
ICON        += SceneQuery.icns
QMAKE_INFO_PLIST = Info.plist
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
PQXX = /usr/local/Cellar/libpqxx/4.0.1_2

INCLUDEPATH += \
    /usr/local/include \
    src \
    units \
    src-threads \
    src-views \
    forms \
    PSQL/src \
    -I/usr/local/opt/qt/bin \
    $${PQXX}/src \
    $${PQXX}/include

PKGCONFIG  = /usr/local/opt/qt/lib/pkgconfig

LIBS += \
    -lcurl \
    -L/usr/local/lib \
    -framework DiskArbitration \
    -framework IOKit \
    -framework OpenGL \
    -framework AGL  \
    -L/usr/local/opt/qt/lib \
    -L/System/Library/Frameworks/ImageIO.framework/Versions/A/Resources \
    -L$${PQXX} \
    -lpqxx \
    -lpq

SOURCES += \
    src/config.cpp \
    src-threads/curlTool.cpp \
    src-threads/FileScanner.cpp \
    src/genericfunctions.cpp \
    src/main.cpp\
    src/mainwindow.cpp \
    src/sceneParser.cpp \
    units/Actor.cpp \
    units/Biography.cpp \
    units/FilePath.cpp \
    units/Height.cpp \
    units/Rating.cpp \
    units/Scene.cpp \
    qtcurl/QtCUrl.cpp \
    PSQL/src/query.cpp \
    PSQL/src/sqlconnection.cpp \
    src-threads/sql.cpp \
    units/Entry.cpp \
    src/filenames.cpp \
    src-views/ActorTableModel.cpp \
    units/SceneList.cpp \
    src-views/profiledialog.cpp \
    src-threads/InitializationThread.cpp \
    src-views/RatingDelegate.cpp \
    src-views/RatingEditor.cpp \
    src-views/SceneProxyModel.cpp \
    src-views/SceneView.cpp \
    forms/ActorProfileView.cpp \
    src-threads/VideoPlayer.cpp

HEADERS  += \
    src/config.h \
    src-threads/curlTool.h \
    src/definitions.h \
    src-threads/FileScanner.h \
    src/genericfunctions.h \
    src/mainwindow.h \
    src/sceneParser.h \
    units/Actor.h \
    units/Biography.h \
    units/FilePath.h \
    units/Height.h \
    units/Rating.h \
    units/Scene.h \
    qtcurl/QtCUrl.h \
    PSQL/src/query.h \
    src-threads/sql.h \
    PSQL/src/sqlconnection.h \
    PSQL/src/sql_definitions.h \
    PSQL/src/database.h \
    units/Entry.h \
    src/filenames.h \
    src-views/ActorTableModel.h \
    units/SceneList.h \
    src-views/profiledialog.h \
    src-threads/InitializationThread.h \
    src-views/RatingDelegate.h \
    src-views/RatingEditor.h \
    src-views/SceneProxyModel.h \
    src-views/SceneView.h \
    forms/ActorProfileView.h \
    src-threads/VideoPlayer.h

FORMS    += forms/mainwindow.ui \
    forms/profiledialog.ui \
    forms/actorprofileview.ui

QMAKE_CLEAN += \
    build/* \
    QSceneQuery.pro.user \
    Makefile

STATECHARTS += \
    forms/GUI_State_chart.scxml

DISTFILES += \
    scripts/collect_exif.sh \
    resources/scripts/collect_exif.sh

RESOURCES += \
    resources/icons.qrc \
    resources/scripts.qrc
