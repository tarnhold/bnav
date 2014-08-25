! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = lib
TARGET = bnav

LIBS += -lboost_date_time

SOURCES += \
    AsciiReader.cpp \
    Subframe.cpp \
    SvID.cpp \
    AsciiReaderEntry.cpp \
    SubframeBufferStore.cpp \
    SubframeBufferD1.cpp \
    SubframeBuffer.cpp \
    SubframeBufferD2.cpp \
    Benchmark.cpp \
    Ionosphere.cpp \
    IonosphereStore.cpp \
    Ephemeris.cpp \
    DateTime.cpp

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
    Benchmark.h \
    Ionosphere.h \
    IonosphereStore.h \
    Ephemeris.h \
    DateTime.h

