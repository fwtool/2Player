#-------------------------------------------------
#
# Project created by QtCreator 2019-03-30T00:23:26
#
#-------------------------------------------------

QT       -= core gui

TARGET = PlaySDK
TEMPLATE = lib

DEFINES += PLAYSDK_LIBRARY

SOURCES += \
    audiodecoder.cpp \
    avpacketqueue.cpp \
    decoder.cpp \
    Player.cpp \
    SDLEngine.cpp

HEADERS +=\
    playsdk_global.h \
    audiodecoder.h \
    avpacketqueue.h \
    decoder.h \
    inc.h \
    SDLDev.h \
    SDLEngine.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH +=  \
    ../inc \
    ../inc/util \
    $$PWD/../inc/ffmpeg \
    $$PWD/../inc/sdl2

LIBS    += \
    $$PWD/../lib/ffmpeg/avcodec.lib \
    $$PWD/../lib/ffmpeg/avdevice.lib \
    $$PWD/../lib/ffmpeg/avfilter.lib \
    $$PWD/../lib/ffmpeg/avformat.lib \
    $$PWD/../lib/ffmpeg/avutil.lib \
    $$PWD/../lib/ffmpeg/postproc.lib \
    $$PWD/../lib/ffmpeg/swresample.lib \
    $$PWD/../lib/ffmpeg/swscale.lib \
    $$PWD/../lib/SDL2.lib

DESTDIR = $$PWD/../bin

CONFIG(debug, debug|release) {
TARGET  = $$join(TARGET,,,d)

LIBS    += -lutild
}else{
LIBS    += -lutil
}

LIBS    += -L$$PWD/../bin

CONFIG += c++11
