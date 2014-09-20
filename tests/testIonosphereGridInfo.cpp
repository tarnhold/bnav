#include <unittest++/UnitTest++.h>
#include "TestConfig.h"

#include "IonosphereGridInfo.h"

#include "AsciiReader.h"
#include "BeiDou.h"
#include "NavBits.h"
#include "SvID.h"

#include <iostream>

SUITE(testIonoGridInfo_Constructor)
{
    // Test empty contructor and max/min allowed values
    TEST(testIonoGridInfo_ConstructorEmpty)
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
    TEST(testIonoGridInfo_ConstructorNavBits)
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

    TEST(testIonoGridInfo_ConstructorVDRMS)
    {
        // vertdelay, rms
        {
            bnav::IonoGridInfo igi(0);
            CHECK(igi.getVerticalDelay_TECU() == 0);
            CHECK(igi.getGive_TECU() == 0);
        }
        {
            bnav::IonoGridInfo igi(9999);
            CHECK(igi.getVerticalDelay_TECU() == 9999);
            CHECK(igi.getGive_TECU() == 0);
        }
        {
            bnav::IonoGridInfo igi(20, 99);
            CHECK(igi.getVerticalDelay_TECU() == 20);
            CHECK(igi.getGive_TECU() == 99);
        }
        {
            bnav::IonoGridInfo igi(9999, 9999);
            CHECK(igi.getVerticalDelay_TECU() == 9999);
            CHECK(igi.getGive_TECU() == 9999);
        }
    }
}

SUITE(testIonoGridInfo_Various)
{
    TEST(testIonoGridInfo_load)
    {
        // vertdelay, rms
        {
            bnav::IonoGridInfo igi;
            igi.load(9999);
            CHECK(igi.getVerticalDelay_TECU() == 9999);
            CHECK(igi.getGive_TECU() == 0);
            igi.load(0, 9999);
            CHECK(igi.getVerticalDelay_TECU() == 0);
            CHECK(igi.getGive_TECU() == 9999);
            igi.load(345, 678);
            CHECK(igi.getVerticalDelay_TECU() == 345);
            CHECK(igi.getGive_TECU() == 678);
        }
    }

    TEST(testIonoGridInfo_setVerticalDelay)
    {
        bnav::IonoGridInfo igi(100, 101);
        CHECK(igi.getVerticalDelay_TECU() == 100);
        CHECK(igi.getGive_TECU() == 101);
        igi.setVerticalDelay_TECU(9999);
        CHECK(igi.getVerticalDelay_TECU() == 9999);
        CHECK(igi.getGive_TECU() == 101);
        igi.setVerticalDelay_TECU(0);
        CHECK(igi.getVerticalDelay_TECU() == 0);
        CHECK(igi.getGive_TECU() == 101);
    }

    // Check if all Vertical Delay values convert correctly TECU
    TEST(testIonoGridInfoVerticalDelayTable)
    {
        bnav::NavBits<13> bits(0);
        bnav::IonoGridInfo igp;

        // Vert delay has 9 bits, this means 512 possible values
        for (std::size_t i = 0; i <= 511; ++i)
        {
            // Vertical delay: steps are 7.55902e-01 TECU
            double vertdelay = i * 7.55902;
            if (i >= 510)
                vertdelay = 9999.0;

            bits = bnav::NavBits<13>(i);
            // move 4 left, because we set only 9 bits maximum
            // vertical delay is left of givei bits
            bits <<= 4;
            igp.load(bits);
            CHECK(igp.getVerticalDelay_TECU() == static_cast<uint32_t>(std::lround(vertdelay)));
            // GIVEI 0 is 0.3 [m] which are 18e-1 TECU
            CHECK(igp.getGive_TECU() == 18);
        }
    }

    // Check if all GIVEI values convert correctly TECU
    TEST(testIonoGridInfoGIVEITable)
    {
        bnav::NavBits<13> bits(0);
        bnav::IonoGridInfo igp;

        // GIVEI table converted in 0.1 TECU
        constexpr uint32_t givei_values[] = {18, 36, 54, 73, 91, 109, 127, 145, 163,
                                         181, 218, 272, 363, 544, 907, 9999};

        // GIVEI has 4 bits, this means 16 possible values
        for (std::size_t i = 0; i <= 15; ++i)
        {
            bits = bnav::NavBits<13>(i);
            igp.load(bits);
            CHECK(igp.getVerticalDelay_TECU() == 0);
            CHECK(igp.getGive_TECU() == givei_values[i]);
        }
    }
}

SUITE(testIonoGridInfo_Operators)
{
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
        // different vertdelay
        {
            bnav::IonoGridInfo igi1(100, 100);
            bnav::IonoGridInfo igi2(100, 100);
            CHECK(igi1 == igi2);
            igi2.setVerticalDelay_TECU(9999);
            CHECK(!(igi1 == igi2));
        }
        // different rms
        {
            bnav::IonoGridInfo igi1(100, 9999);
            bnav::IonoGridInfo igi2(100, 100);
            CHECK(!(igi1 == igi2));
            igi2.load(100, 9999);
            CHECK(igi1 == igi2);
        }
        // all 9999
        {
            bnav::IonoGridInfo igi1(9999, 9999);
            bnav::IonoGridInfo igi2(9999, 9999);
            CHECK(igi1 == igi2);
        }
    }

    TEST(testIonoGridInfo_operatorMinus)
    {
        // simple one
        {
            bnav::IonoGridInfo igi1(100, 100);
            bnav::IonoGridInfo igi2(50, 50);
            bnav::IonoGridInfo igim = igi1 - igi2;
            CHECK(igim.getVerticalDelay_TECU() == 50);
            CHECK(igim.getGive_TECU() == 50);

            igim = igi2 - igi1;
            CHECK(igim.getVerticalDelay_TECU() == 50);
            CHECK(igim.getGive_TECU() == 50);
        }
        // same values
        {
            bnav::IonoGridInfo igi1(333, 333);
            bnav::IonoGridInfo igi2(333, 333);
            bnav::IonoGridInfo igim = igi1 - igi2;
            CHECK(igim.getVerticalDelay_TECU() == 0);
            CHECK(igim.getGive_TECU() == 0);

            igim = igi2 - igi1;
            CHECK(igim.getVerticalDelay_TECU() == 0);
            CHECK(igim.getGive_TECU() == 0);
        }
        {
            // first vertdelay 9999
            {
                bnav::IonoGridInfo igi1(9999);
                bnav::IonoGridInfo igi2(100);
                bnav::IonoGridInfo igim = igi1 - igi2;
                CHECK(igim.getVerticalDelay_TECU() == 9999);
                CHECK(igim.getGive_TECU() == 0);

                igim = igi2 - igi1;
                CHECK(igim.getVerticalDelay_TECU() == 9999);
                CHECK(igim.getGive_TECU() == 0);
            }
            // second vertdelay 9999
            {
                bnav::IonoGridInfo igi1(100);
                bnav::IonoGridInfo igi2(9999);
                bnav::IonoGridInfo igim = igi1 - igi2;
                CHECK(igim.getVerticalDelay_TECU() == 9999);
                CHECK(igim.getGive_TECU() == 0);

                igim = igi2 - igi1;
                CHECK(igim.getVerticalDelay_TECU() == 9999);
                CHECK(igim.getGive_TECU() == 0);
            }
            // both vertdelay 9999
            {
                bnav::IonoGridInfo igi1(9999);
                bnav::IonoGridInfo igi2(9999);
                bnav::IonoGridInfo igim = igi1 - igi2;
                CHECK(igim.getVerticalDelay_TECU() == 9999);
                CHECK(igim.getGive_TECU() == 0);

                igim = igi2 - igi1;
                CHECK(igim.getVerticalDelay_TECU() == 9999);
                CHECK(igim.getGive_TECU() == 0);
            }
        }
        {
            // first rms 9999
            {
                bnav::IonoGridInfo igi1(0, 9999);
                bnav::IonoGridInfo igi2(0, 20);
                bnav::IonoGridInfo igim = igi1 - igi2;
                CHECK(igim.getVerticalDelay_TECU() == 0);
                CHECK(igim.getGive_TECU() == 9999);

                igim = igi2 - igi1;
                CHECK(igim.getVerticalDelay_TECU() == 0);
                CHECK(igim.getGive_TECU() == 9999);
            }
            // second rms 9999
            {
                bnav::IonoGridInfo igi1(100, 9999);
                bnav::IonoGridInfo igi2(100, 9999);
                bnav::IonoGridInfo igim = igi1 - igi2;
                CHECK(igim.getVerticalDelay_TECU() == 0);
                CHECK(igim.getGive_TECU() == 9999);

                igim = igi2 - igi1;
                CHECK(igim.getVerticalDelay_TECU() == 0);
                CHECK(igim.getGive_TECU() == 9999);
            }
            // both rms 9999
            {
                bnav::IonoGridInfo igi1(9999, 9999);
                bnav::IonoGridInfo igi2(9999, 9999);
                bnav::IonoGridInfo igim = igi1 - igi2;
                CHECK(igim.getVerticalDelay_TECU() == 9999);
                CHECK(igim.getGive_TECU() == 9999);

                igim = igi2 - igi1;
                CHECK(igim.getVerticalDelay_TECU() == 9999);
                CHECK(igim.getGive_TECU() == 9999);
            }
        }
    }
}
