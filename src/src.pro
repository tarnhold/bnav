! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = app
TARGET = bapp

CONFIG += console
CONFIG -= app_bundle

LIBS += -L../lib -lbnav -lboost_program_options -lboost_regex

# libbnav lives in ../lib relative to this binary. $ORIGIN keeps that
# working without LD_LIBRARY_PATH, including after the build tree is moved.
unix:!macx {
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGIN/../lib\'
}

SOURCES += main.cpp \
    bnavMain.cpp

HEADERS += \
    bnavMain.h

OTHER_FILES +=

