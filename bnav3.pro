TEMPLATE = subdirs

SUBDIRS += \
    tests \
    src \
    lib

app.depends = lib
tests.depends = lib
