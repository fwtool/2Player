#-------------------------------------------------
#
# Project created by QtCreator 2019-03-27T01:29:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 2Player
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH += \
    ../inc \
    ../inc/util

DESTDIR = $$PWD/../bin

CONFIG(debug, debug|release) {
TARGET  = $$join(TARGET,,,d)

LIBS    += -lutild -lMediaLibd -lmodeld
}else{
LIBS    += -lutil -lMediaLib -lmodel
}

LIBS    += -L$$PWD/../bin

CONFIG += c++11
