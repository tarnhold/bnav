! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = app
TARGET = bapp

CONFIG += console
CONFIG -= app_bundle

SOURCES += main.cpp

HEADERS +=

LIBS += -L../lib -lbnav

OTHER_FILES +=

