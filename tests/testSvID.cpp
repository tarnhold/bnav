#include <UnitTest++/UnitTest++.h>
#include "TestConfig.h"

#include "SvID.h"

SUITE(testSvID)
{
    // constructor tests cover: isGeo, getPRN, setPRN
    // empty constructor
    TEST(testSvID_Constructor1)
    {
        bnav::SvID sv;

        // default should be not valid
        CHECK_EQUAL(0, sv.getPRN());
        CHECK(!sv.isGeo());

        // check all possible PRNs
        for (std::size_t i = 1; i <= bnav::BDS_MAX_PRN; ++i)
        {
            sv.setPRN(i);
            CHECK_EQUAL(i, sv.getPRN());

            if (i <= 5)
                CHECK(sv.isGeo());
            else
                CHECK(!sv.isGeo());
        }
    }

    TEST(testSvID_Constructor2)
    {
        // check all possible PRNs
        for (std::size_t i = 1; i <= bnav::BDS_MAX_PRN; ++i)
        {
            bnav::SvID sv(i);
            CHECK_EQUAL(i, sv.getPRN());

            if (i <= 5)
                CHECK(sv.isGeo());
            else
                CHECK(!sv.isGeo());
        }
    }

    // both operator== and operator!=
    TEST(testSvID_operatorEqual)
    {
        bnav::SvID sv1(1);
        bnav::SvID sv2(1);
        CHECK(sv1 == sv2);
        CHECK(!(sv1 != sv2));
        sv1.setPRN(2);
        CHECK(!(sv1 == sv2));
        CHECK(sv1 != sv2);
    }

    TEST(testSvID_operatorLess)
    {
        bnav::SvID sv1(1);
        bnav::SvID sv2(1);
        CHECK(!(sv1 < sv2));
        sv2.setPRN(2);
        CHECK(sv1 < sv2);
        sv1.setPRN(3);
        CHECK(!(sv1 < sv2));
    }
}
