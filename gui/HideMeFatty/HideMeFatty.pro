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
		-lcrypto++

SOURCES += main.cpp \
        mainwindow.cpp \
    ../../src/DirectoryEntry.cpp \
    ../../src/DistributedMemoryMapper.cpp \
    ../../src/Fat32Manager.cpp \
    ../../src/FatStructs.cpp \
    ../../src/FileHider.cpp \
    ../../src/MappedFileManager.cpp

QMAKE_CXXFLAGS += -std=c++11

HEADERS  += mainwindow.h \
    ../../include/PartitionFinder.hpp \
    ../../include/DirectoryEntry.hpp \
    ../../include/DistributedMemoryMapper.hpp \
    ../../include/Fat32Manager.hpp \
    ../../include/FatStructs.h \
    ../../include/FileHider.hpp \
    ../../include/MappedFileManager.hpp \
    ../../include/pathOperations.hpp \
    ../../include/Structs.h \
    ../../include/FileTable.hpp \
    ../../include/PartitionInfo.hpp \
    ../../include/FilesToHideTable.hpp \
    ../../include/HideFilesOnPartitionTable.hpp


FORMS    += mainwindow.ui
