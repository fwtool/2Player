#-------------------------------------------------
#
# Project created by QtCreator 2019-03-27T06:30:38
#
#-------------------------------------------------

QT       -= core gui

TARGET = util
TEMPLATE = lib

DEFINES += UTIL_LIBRARY

SOURCES += \
    sqlite/shell.c \
    sqlite/sqlite3.c \
    fsdlg.cpp \
    fsutil.cpp \
    Path.cpp \
    ProFile.cpp \
    SQLiteDB.cpp \
    util.cpp

HEADERS +=\
        util_global.h \
    sqlite/sqlite3.h \
    sqlite/sqlite3ext.h \
    ../inc/util/fsdlg.h \
    ../inc/util/fsutil.h \
    ../inc/util/IDB.h \
    ../inc/util/mtutil.h \
    ../inc/util/Path.h \
    ../inc/util/ProFile.h \
    ../inc/util/SQLiteDB.h \
    ../inc/util/util.h \
    ../inc/util/WorkThread.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DEFINES += __UtilPrj

INCLUDEPATH += \
    ../inc \
    ../inc/util

DESTDIR = $$PWD/../bin
CONFIG(debug, debug|release) : TARGET  = $$join(TARGET,,,d)

CONFIG += c++11
