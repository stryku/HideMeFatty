#-------------------------------------------------
#
# Project created by QtCreator 2015-04-01T21:27:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HideMeFatty
TEMPLATE = app

INCLUDEPATH += ../../include

LIBS += -lboost_system \
        -lboost_filesystem \
        -lboost_iostreams \
        -lboost_timer \
        -lcrypto++

SOURCES += main.cpp \
        mainwindow.cpp \
    ../../src/DirectoryEntry.cpp \
    ../../src/Fat32Manager.cpp \
    ../../src/FatStructs.cpp \
    ../../src/FileHider.cpp \
    ../../src/MappedFileManager.cpp \
    ../../src/FileTable.cpp \
    ../../src/FilesToHideTable.cpp \
    ../../src/HideFilesOnPartitionTable.cpp \
    ../../src/HideSectionFileTable.cpp \
    ../../src/PartitionFinder.cpp \
    ../../src/PartitionInfo.cpp \
    ../../src/DistributedMemoryManager.cpp \
    ../../src/Task.cpp \
    ../../src/Preparator.cpp \
    ../../src/PreparatorToHide.cpp \
    ../../src/PreparatorToRestore.cpp \
    ../../src/TaskTree.cpp

QMAKE_CXXFLAGS += -std=c++11

HEADERS  += mainwindow.h \
    ../../include/PartitionFinder.hpp \
    ../../include/DirectoryEntry.hpp \
    ../../include/Fat32Manager.hpp \
    ../../include/FatStructs.h \
    ../../include/FileHider.hpp \
    ../../include/MappedFileManager.hpp \
    ../../include/Structs.h \
    ../../include/FileTable.hpp \
    ../../include/PartitionInfo.hpp \
    ../../include/FilesToHideTable.hpp \
    ../../include/HideFilesOnPartitionTable.hpp \
    ../../include/RestoreFilesOnPartitionTable.hpp \
    ../../include/HideSectionFileTable.hpp \
    ../../include/DistributedMemoryManager.hpp \
    ../../include/TaskTree.hpp \
    ../../include/Task.hpp \
    ../../include/Preparator.hpp \
    ../../include/PreparatorToHide.hpp \
    ../../include/PreparatorToRestore.hpp


FORMS    += mainwindow.ui

OTHER_FILES += \
    ../../TODO.txt

CONFIG += static

INCLUDEPATH += $$PWD/../../../../../libraries/boost/lib/static
DEPENDPATH += $$PWD/../../../../../libraries/boost/lib/static

unix:!macx: LIBS += -L$$PWD/../../../../../libraries/boost/lib/static/ -lboost_filesystem
unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../libraries/boost/lib/static/libboost_filesystem.a

unix:!macx: LIBS += -L$$PWD/../../../../../libraries/boost/lib/static/ -lboost_iostreams
unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../libraries/boost/lib/static/libboost_iostreams.a

unix:!macx: LIBS += -L$$PWD/../../../../../libraries/boost/lib/static/ -lboost_system
unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../libraries/boost/lib/static/libboost_system.a

unix:!macx: LIBS += -L$$PWD/../../../../../libraries/boost/lib/static/ -lboost_timer
unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../libraries/boost/lib/static/libboost_timer.a

unix:!macx: LIBS += -L$$PWD/../../../../../libraries/boost/lib/static/ -lcrypto++
unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../libraries/boost/lib/static/libcrypto++.a

unix:!macx: LIBS += -L$$PWD/../../../../../libraries/boost/lib/static/ -lboost_chrono

INCLUDEPATH += $$PWD/../../../../../libraries/boost/lib/static
DEPENDPATH += $$PWD/../../../../../libraries/boost/lib/static

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../libraries/boost/lib/static/libboost_chrono.a
