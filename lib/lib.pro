! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = lib
TARGET = bnav

SOURCES += \
    AsciiReader.cpp \
    NavBits.cpp \
    Subframe.cpp \
    AsciiReaderNavEntry.cpp \
    NavBitsECC.cpp

HEADERS += \
    AsciiReader.h \
    Debug.h \
    NavBits.h \
    Subframe.h \
    AsciiReaderNavEntry.h \
    NavBitsECC.h \
    BDSCommon.h

