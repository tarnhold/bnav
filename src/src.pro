! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = app
TARGET = bapp

CONFIG += console
CONFIG -= app_bundle

LIBS += -L../lib -lbnav

SOURCES += main.cpp

HEADERS +=

OTHER_FILES +=

