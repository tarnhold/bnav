# Includes common configuration for all subdirectory .pro files.

# No Qt types are used. Drop the modules qmake would otherwise require
# and link (this fails on a machine that has qmake but not Qt).
CONFIG -= qt
QT -= core gui widgets

# ISO C++23. Qt 6.4 qmake selects the standard through CONFIG+=c++2b and,
# unless strict_c++, prefers gnu++2b. Override the flag so the compiler
# is invoked with -std=c++23 (not gnu++2b).
CONFIG += c++2b strict_c++
QMAKE_CXXFLAGS_CXX2B = -std=c++23

INCLUDEPATH += $$PWD/lib

QMAKE_CXXFLAGS += -Wall \
        -Wextra \
        -Wpedantic \
        -Wendif-labels \
        -Wundef \
        -Wunused-macros \
        -Wfloat-equal \
        -Wshadow \
        -Wsign-conversion \
        -Wconversion \
        -Wdeprecated \
        -Wno-padded \
        -fmessage-length=0 \
        -fno-common

# clang specific warnings
clang {
QMAKE_CXXFLAGS += -Wdocumentation \
        -Wno-c++98-compat \
        -Wno-c++98-compat-pedantic \
}
