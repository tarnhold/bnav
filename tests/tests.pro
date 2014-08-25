! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle

SOURCES += main.cpp \
    testNavBits.cpp \
    testAsciiReader.cpp \
    testNavBitsECC.cpp \
    testSubframe.cpp \
    testSvID.cpp \
    testSubframeBuffer.cpp \
    testSubframeBufferStore.cpp \
    testIonosphere.cpp

LIBS += -lunittest++ -L../lib -lbnav

HEADERS += \
    TestConfig.h
