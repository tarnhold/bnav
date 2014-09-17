! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = app
TARGET = bapp

CONFIG += console
CONFIG -= app_bundle

LIBS += -L../lib -lbnav -lboost_program_options -lboost_regex

SOURCES += main.cpp \
    bnavMain.cpp

HEADERS += \
    bnavMain.h

OTHER_FILES +=

