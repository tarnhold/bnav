! include( ../defaults.pri ) {
    error( "Couldn't find the defaults.pri file!" )
}

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle

LIBS += -lUnitTest++ -L../lib -lbnav

unix:!macx {
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGIN/../lib\'
}

# Absolute path to tests/data, independent of the process working directory.
DEFINES += "BNAV_PATH_TESTDATA=\\\"$$PWD/data\\\""

SOURCES += main.cpp \
    testNavBits.cpp \
    testAsciiReader.cpp \
    testNavBitsECC.cpp \
    testSubframe.cpp \
    testSvID.cpp \
    testSubframeBuffer.cpp \
    testSubframeBufferStore.cpp \
    testIonosphere.cpp \
    testDateTime.cpp \
    testSamples.cpp \
    testEphemeris.cpp \
    testIonosphereGridInfo.cpp

HEADERS += \
    TestConfig.h
