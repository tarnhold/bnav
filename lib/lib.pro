! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = lib
TARGET = bnav

SOURCES += \
    AsciiReader.cpp \
    NavBits.cpp

HEADERS += \
    AsciiReader.h \
    Debug.h \
    NavBits.h

