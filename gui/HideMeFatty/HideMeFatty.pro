#-------------------------------------------------
#
# Project created by QtCreator 2015-04-01T21:27:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HideMeFatty
TEMPLATE = app

boost=C:/MOJE/libs/boost/boost_1_57_0
boost_include=$$boost
boost_libs = $$boost/x64/libboost_filesystem-vc120-mt-1_57.lib

INCLUDEPATH += $$boost_include

SOURCES += main.cpp \
        mainwindow.cpp


LIBS += $$boost_libs

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
