! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = lib
TARGET = bnav

SOURCES += \
    AsciiReader.cpp \
    NavBits.cpp \
    Subframe.cpp \
    NavBitsECC.cpp \
    AsciiReaderNavEntry.cpp

HEADERS += \
    AsciiReader.h \
    Debug.h \
    NavBits.h \
    Subframe.h \
    NavBitsECC.h \
    AsciiReaderNavEntry.h

