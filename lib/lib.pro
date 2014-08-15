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
    AsciiReaderEntry.cpp \
    SubframeBufferStore.cpp \
    SubframeBufferD1.cpp \
    SubframeBuffer.cpp \
    SubframeBufferD2.cpp \
    Benchmark.cpp

HEADERS += \
    AsciiReader.h \
    Debug.h \
    NavBits.h \
    Subframe.h \
    NavBitsECC.h \
    SvID.h \
    AsciiReaderEntry.h \
    BeiDou.h \
    SubframeBufferStore.h \
    SubframeBuffer.h \
    Benchmark.h

