TEMPLATE = subdirs

# The project does not use Qt. Without this, qmake requires the Qt
# core/gui modules and links them into every target.
CONFIG -= qt
QT -= core gui widgets

SUBDIRS += \
    lib \
    src \
    tests

src.depends = lib
tests.depends = lib
