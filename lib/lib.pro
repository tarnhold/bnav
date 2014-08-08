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
    SvID.cpp \
    SubframeBuffer.cpp \
    AsciiReaderEntry.cpp

HEADERS += \
    AsciiReader.h \
    Debug.h \
    NavBits.h \
    Subframe.h \
    NavBitsECC.h \
    SvID.h \
    SubframeBuffer.h \
    AsciiReaderEntry.h \
    BeiDou.h

