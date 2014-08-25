#Includes common configuration for all subdirectory .pro files.
INCLUDEPATH += $$PWD/lib

QMAKE_CXXFLAGS += -std=c++11 \
        -Wendif-labels \
        -Wextra \
        -Wundef \
        -Wunused-macros \
        -Wfloat-equal \
        -Wshadow \
        -Wsign-conversion \
        -Wconversion \
        -Wdeprecated \
        -Wdocumentation \
        #-Weverything \
        -Wno-c++98-compat \
        -fmessage-length=0 \
        -fno-common \

HEADERS +=

SOURCES +=
