#include <unittest++/UnitTest++.h>
#include "TestConfig.h"

#include "Ionosphere.h"

#include "AsciiReader.h"
#include "BeiDou.h"
#include "NavBits.h"
#include "SvID.h"

#include <iostream>

SUITE(testIonosphere)
{
    // Test empty contructor and max/min allowed values
    TEST(testIonoGridInfoCtorEmpty)
    {
        bnav::NavBits<13> bits("0000000000000");
        bnav::IonoGridInfo igp;
        igp.load(bits);

        // minimum values, dt=0.0m, GIVEI 0
        CHECK(igp.getVerticalDelay_TECU() == 0);
        // GIVEI 0 is 0.3 m which are 18 TECU
        CHECK(igp.getGive_TECU() == 18);

        // maximum values, dt=63.875m, GIVEI 15
        bits = bnav::NavBits<13>("1111111111111");
        igp.load(bits);
        CHECK(igp.getVerticalDelay_TECU() == 9999);
        CHECK(igp.getGive_TECU() == 9999);

        // dt=63.625m, GIVEI 15
        bits = bnav::NavBits<13>("1111111101111");
        igp.load(bits);
        CHECK(igp.getVerticalDelay_TECU() == 9999);
        CHECK(igp.getGive_TECU() == 9999);
    }

    // Test constructor
    TEST(testIonoGridInfoCtor)
    {
        bnav::NavBits<13> bits("0000000010001");
        bnav::IonoGridInfo igp(bits);

        // first values
        // minimum values, dt=0.125m, GIVEI 1
        CHECK(igp.getVerticalDelay_TECU() == 8);
        // GIVEI 0 is 0.3 m which are 18 TECU
        CHECK(igp.getGive_TECU() == 36);

        // last values
        // dt=63.5m, GIVEI 14
        bits = bnav::NavBits<13>("1111111001110");
        igp.load(bits);
        CHECK(igp.getVerticalDelay_TECU() == 3840);
        CHECK(igp.getGive_TECU() == 907);
    }

    // Check if all GIVEI values convert correctly TECU
    TEST(testIonoGridInfoGIVEITable)
    {
        bnav::NavBits<13> bits(0);
        bnav::IonoGridInfo igp;

        // GIVEI table converted in 0.1 TECU
        constexpr uint32_t givei_values[] = {18, 36, 54, 73, 91, 109, 127, 145, 163,
                                         181, 218, 272, 363, 544, 907, 9999};

        for (std::size_t i = 0; i <= 15; ++i)
        {
            bits = bnav::NavBits<13>(i);
            igp.load(bits);
            CHECK(igp.getVerticalDelay_TECU() == 0);
            CHECK(igp.getGive_TECU() == givei_values[i]);
        }
    }

    // Test comparision of two objects
    TEST(testIonoGridInfoOperatorEqual)
    {
        {
            bnav::NavBits<13> bits(0);
            bnav::IonoGridInfo igp1(bits);
            bnav::IonoGridInfo igp2(bits);
            CHECK(igp1 == igp2);
        }
        {
            bnav::NavBits<13> bits(511);
            bnav::IonoGridInfo igp1(bits);
            bnav::IonoGridInfo igp2(bits);
            CHECK(igp1 == igp2);
        }
        {
            bnav::NavBits<13> bits("1111111001110");
            bnav::IonoGridInfo igp1(bits);
            bnav::IonoGridInfo igp2(bits);
            CHECK(igp1.getVerticalDelay_TECU() == 3840);
            CHECK(igp1.getGive_TECU() == 907);
            CHECK(igp1 == igp2);

            bnav::IonoGridInfo igp3(bnav::NavBits<13>(0));
            CHECK(!(igp3 == igp1));
        }
    }
}
