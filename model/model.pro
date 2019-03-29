#-------------------------------------------------
#
# Project created by QtCreator 2019-03-27T22:24:54
#
#-------------------------------------------------

QT       -= gui

TARGET = model
TEMPLATE = lib

DEFINES += MODEL_LIBRARY

SOURCES += \
    Dao.cpp \
    AutoMatch.cpp \
    BackupMgr.cpp \
    DataMgr.cpp \
    DBCompare.cpp \
    MediaMixer.cpp \
    MediaObserver.cpp \
    model.cpp \
    PlaylistMgr.cpp \
    PlayMgr.cpp \
    SimilarCheck.cpp \
    SingerMgr.cpp

HEADERS +=\
        model_global.h \
    ../inc/Dao.h \
    DBCompare.h \
    MediaObserver.h \
    SimilarCheck.h \
    ../inc/AutoMatch.h \
    ../inc/BackupMgr.h \
    ../inc/CueFile.h \
    ../inc/DataMgr.h \
    ../inc/IMedia.h \
    ../inc/Media.h \
    ../inc/MediaDef.h \
    ../inc/MediaMixer.h \
    ../inc/MediaMixture.h \
    ../inc/MediaRes.h \
    ../inc/MediaSet.h \
    ../inc/model.h \
    ../inc/PlaylistMedia.h \
    ../inc/PlaylistMgr.h \
    ../inc/PlayMgr.h \
    ../inc/RootMediaRes.h \
    ../inc/SingerMedia.h \
    ../inc/SingerMgr.h \
    ../inc/viewdef.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DEFINES += __DalPrj __ModelPrj

INCLUDEPATH += \
    ../inc \
    ../inc/util

DESTDIR = $$PWD/../bin

CONFIG(debug, debug|release) {
TARGET  = $$join(TARGET,,,d)

LIBS    += -lutild -lMediaLibd -lPlaySDKd
}else{
LIBS    += -lutil -lMediaLib -lPlaySDK
}

LIBS    += -L$$PWD/../bin

CONFIG += c++11
