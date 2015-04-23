#-------------------------------------------------
#
# Project created by QtCreator 2015-04-01T21:27:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HideMeFatty
TEMPLATE = app

LIBS += -lboost_system \
        -lboost_filesystem

SOURCES += main.cpp \
        mainwindow.cpp

QMAKE_CXXFLAGS += -std=c++11

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
