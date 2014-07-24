! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle

SOURCES += main.cpp

HEADERS +=

LIBS += -L../lib -lbnav

OTHER_FILES +=

