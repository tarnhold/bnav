#ifndef TESTCONFIG_H
#define TESTCONFIG_H

#include <string>

// Set by tests/tests.pro from $$PWD so the suite does not depend on a
// particular checkout path. tests.pro appends no trailing slash; add one
// here so existing PATH_TESTDATA + "relative/file" concatenations work.
#ifndef BNAV_PATH_TESTDATA
#error "BNAV_PATH_TESTDATA must be defined by tests/tests.pro"
#endif

const std::string PATH_TESTDATA{std::string(BNAV_PATH_TESTDATA) + "/"};

#endif // TESTCONFIG_H
