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

PRECOMPILED_HEADER = src/precompiledheaders.h

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
    src/Threads \
    forms \
    forms/Delegates \
    forms/Dialogs \
    forms/ItemViews \
    src-objects \
    src \
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
    src/config.cpp \
    src/genericfunctions.cpp \
    src/main.cpp\
    src/filenames.cpp \
    src/SceneRenamer.cpp \
    src/SceneParser.cpp \
    src/DataManager.cpp \
    src-objects/Filepath.cpp \
    src-objects/FilterSet.cpp \
    src-objects/Actor.cpp \
    src-objects/Biography.cpp \
    src-objects/Entry.cpp \
    src-objects/Height.cpp \
    src-objects/Rating.cpp \
    src-objects/Scene.cpp \
    src-objects/SceneList.cpp \
    src/Threads/curlTool.cpp \
    src/Threads/FileScanner.cpp \
    src/Threads/MiniThreads.cpp \
    src/Threads/SQL.cpp \
    PSQL/src/sqlconnection.cpp \
    PSQL/src/Query.cpp \
    forms/RatingEditor.cpp \
    forms/ActorProfileView.cpp \
    forms/Delegates/RatingDelegate.cpp \
    forms/Delegates/DeleteButtonDelegate.cpp \
    forms/Dialogs/profiledialog.cpp \
    forms/Dialogs/VideoPlayer.cpp \
    forms/Dialogs/imageeditor.cpp \
    forms/Dialogs/SplashScreen.cpp \
    ImageCropper/src/imagecropper.cpp \
    forms/ItemViews/SceneProxyModel.cpp \
    forms/ItemViews/ActorProxyModel.cpp \
    forms/ItemViews/ActorTableView.cpp \
    forms/ItemViews/SceneTableView.cpp \
    forms/SceneDetailView.cpp \
    forms/FilterWidget.cpp \
    forms/MainWindow.cpp

HEADERS  += \
    src/config.h \
    src/definitions.h \
    src/filenames.h \
    src/SceneRenamer.h \
    src/SceneParser.h \
    src/DataManager.h \
    src/Exception.h \
    src/precompiledheaders.h \
    src/Threads/FileScanner.h \
    src/Threads/curlTool.h \
    src/Threads/SQL.h \
    src/Threads/MiniThreads.h \
    src/genericfunctions.h \
    src-objects/Actor.h \
    src-objects/Biography.h \
    src-objects/Height.h \
    src-objects/Rating.h \
    src-objects/Scene.h \
    src-objects/Entry.h \
    src-objects/Filepath.h \
    src-objects/FilterSet.h \
    src-objects/SceneList.h \
    PSQL/src/sqlconnection.h \
    PSQL/src/sql_definitions.h \
    PSQL/src/database.h \
    PSQL/src/Query.h \
    forms/RatingEditor.h \
    forms/ActorProfileView.h \
    forms/SceneDetailView.h \
    forms/Delegates/RatingDelegate.h \
    forms/Delegates/DeleteButtonDelegate.h \
    forms/Dialogs/profiledialog.h \
    forms/Dialogs/VideoPlayer.h \
    forms/Dialogs/imageeditor.h \
    forms/Dialogs/SplashScreen.h \
    forms/ItemViews/ActorProxyModel.h \
    forms/ItemViews/ActorTableView.h \
    forms/ItemViews/SceneProxyModel.h \
    forms/ItemViews/SceneTableView.h \
    ImageCropper/src/imagecropper.h \
    forms/FilterWidget.h \
    forms/MainWindow.h

FORMS    += \
    forms/Dialogs/profiledialog.ui \
    forms/ActorProfileView.ui \
    forms/SceneDetailView.ui \
    forms/Dialogs/SplashScreen.ui \
    forms/Dialogs/SearchPathsDialog.ui \
    forms/FilterWidget.ui \
    forms/MainWindow.ui

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
