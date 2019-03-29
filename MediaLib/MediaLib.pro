#-------------------------------------------------
#
# Project created by QtCreator 2019-03-28T20:47:28
#
#-------------------------------------------------

QT       -= core gui

TARGET = MediaLib
TEMPLATE = lib

DEFINES += MEDIALIB_LIBRARY

SOURCES += \
    CueFile.cpp \
    IMedia.cpp \
    Media.cpp \
    MediaMixture.cpp \
    MediaRes.cpp \
    MediaSet.cpp \
    MP3ID3.cpp \
    PlaylistMedia.cpp \
    RootMediaRes.cpp \
    SingerMedia.cpp

HEADERS +=\
    medialib_global.h \
    MP3ID3.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DEFINES += __MediaLibPrj

INCLUDEPATH += \
    ../inc \
    ../inc/util

DESTDIR = $$PWD/../bin

CONFIG(debug, debug|release) {
TARGET  = $$join(TARGET,,,d)

LIBS += -lutild
}else{
LIBS += -lutil
}

LIBS    += -L$$PWD/../bin

CONFIG += c++11
