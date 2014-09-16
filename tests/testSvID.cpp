#include <unittest++/UnitTest++.h>
#include "TestConfig.h"

#include "SvID.h"

// empty constructor
TEST(testSvID)
{
    bnav::SvID sv;

    // default should be not valid
    CHECK(sv.getPRN() == 0);
    CHECK(!sv.isGeo());

    // check all possible PRNs
    for (std::size_t i = 1; i <= bnav::BDS_MAX_PRN; ++i)
    {
        sv.setPRN(i);
        CHECK(sv.getPRN() == i);

        if (i <= 5)
            CHECK(sv.isGeo());
        else
            CHECK(!sv.isGeo());
    }
}

TEST(testSvID2)
{
    // check all possible PRNs
    for (std::size_t i = 1; i <= bnav::BDS_MAX_PRN; ++i)
    {
        bnav::SvID sv(i);
        CHECK(sv.getPRN() == i);

        if (i <= 5)
            CHECK(sv.isGeo());
        else
            CHECK(!sv.isGeo());
    }
}
