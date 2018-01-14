#-------------------------------------------------
#
# Project created by QtCreator 2017-07-31T11:14:28
#
#-------------------------------------------------

QT       += core gui sql concurrent network multimedia widgets multimediawidgets

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
PQXX = /usr/local/Cellar/libpqxx/6.0.0_1

INCLUDEPATH += \
    /usr/local/include \
    src \
    forms \
    src-objects \
    src-helpers \
    PSQL/src \
    -I/usr/local/opt/qt/bin \
    -I$${PQXX}/include/pqxx \
#   libpqxx/include \
 #   libpqxx/src \
    ImageCropper/src

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
#    -Llibpqxx \
    -L$${PQXX} \
    -L$${PQXX}/lib \
    -lpqxx \
    -lpq

SOURCES += \
    src-helpers/config.cpp \
    src/curlTool.cpp \
    src/FileScanner.cpp \
    src-helpers/genericfunctions.cpp \
    src/main.cpp\
    src/mainwindow.cpp \
    src/sceneParser.cpp \
    src-objects/Actor.cpp \
    src-objects/Biography.cpp \
    src-objects/Entry.cpp \
    src-objects/Height.cpp \
    src-objects/Rating.cpp \
    src-objects/Scene.cpp \
    src-objects/SceneList.cpp \
    PSQL/src/query.cpp \
    PSQL/src/sqlconnection.cpp \
    src/sql.cpp \
    src-helpers/filenames.cpp \
    forms/profiledialog.cpp \
    src/InitializationThread.cpp \
    forms/RatingDelegate.cpp \
    forms/RatingEditor.cpp \
    forms/SceneProxyModel.cpp \
    forms/SceneView.cpp \
    forms/ActorProfileView.cpp \
    src/VideoPlayer.cpp \
    forms/imageeditor.cpp \
    ImageCropper/src/imagecropper.cpp \
    forms/ActorProxyModel.cpp \
    forms/ActorTableView.cpp \
    forms/SceneDetailView.cpp \
    src-objects/Filepath.cpp

HEADERS  += \
    src-helpers/config.h \
    src/curlTool.h \
    src/definitions.h \
    src/FileScanner.h \
    src-helpers/genericfunctions.h \
    src/mainwindow.h \
    src/sceneParser.h \
    src-objects/Actor.h \
    src-objects/Biography.h \
    src-objects/Height.h \
    src-objects/Rating.h \
    src-objects/Scene.h \
    PSQL/src/query.h \
    src/sql.h \
    PSQL/src/sqlconnection.h \
    PSQL/src/sql_definitions.h \
    PSQL/src/database.h \
    src-objects/Entry.h \
    src-helpers/filenames.h \
    src-objects/SceneList.h \
    forms/profiledialog.h \
    src/InitializationThread.h \
    forms/RatingDelegate.h \
    forms/RatingEditor.h \
    forms/SceneProxyModel.h \
    forms/SceneView.h \
    forms/ActorProfileView.h \
    src/VideoPlayer.h \
    forms/imageeditor.h \
    ImageCropper/src/imagecropper.h \
    forms/ActorProxyModel.h \
    forms/ActorTableView.h \
    forms/SceneDetailView.h \
    src-objects/Filepath.h

FORMS    += forms/mainwindow.ui \
    forms/profiledialog.ui \
    forms/ActorProfileView.ui \
    forms/SceneDetailView.ui

QMAKE_CLEAN += \
    build/* \
    QSceneQuery.pro.user \
    Makefile

DISTFILES += \
    scripts/collect_exif.sh \
    resources/scripts/collect_exif.sh

RESOURCES += \
    resources/icons.qrc \
    resources/scripts.qrc
