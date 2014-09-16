#include <unittest++/UnitTest++.h>
#include "TestConfig.h"

#include "Subframe.h"
#include "SubframeBuffer.h"

#include "AsciiReader.h"
#include "BeiDou.h"
#include "NavBits.h"
#include "SvID.h"

#include <iostream>

// Test real data
// Only for data sets from one single PRN
SUITE(testSubframeBuffer_SBF_Superframe_OnePRN)
{
    TEST(testSubframeBuffer_SuperframeD1)
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/subframebuffer/CUT12014071724.sbf_SBF_CMPRaw-prn6-onesuperframe.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        bnav::SubframeBufferD1 sfbuf;

        std::size_t msgcount = 0, ephcount = 0, almcount = 0, paritycount = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            CHECK(!sv.isGeo());
            bnav::Subframe sf(sv, entry.getBits());
            paritycount += sf.getParityModifiedCount();

            sfbuf.addSubframe(sf);

            if (sfbuf.isEphemerisComplete())
            {
                sfbuf.clearEphemerisData();
                ++ephcount;
            }

            else if (sfbuf.isAlmanacComplete())
            {
                sfbuf.clearAlmanacData();
                ++almcount;
            }

            ++msgcount;
        }
        CHECK(msgcount == 150);
        // we should have completed 30 ephemeris data sets
        CHECK(ephcount == 30);
        // and 1 almanac data set
        CHECK(almcount == 1);
        CHECK(paritycount == 0);
        reader.close();
    }

    TEST(testSubframeBuffer_SuperframeD2)
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/subframebuffer/CUT12014071724.sbf_SBF_CMPRaw-prn2-onesuperframe.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        bnav::SubframeBufferD2 sfbuf;

        std::size_t msgcount = 0, ephcount = 0, almcount = 0, paritycount = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            CHECK(sv.isGeo());
            bnav::Subframe sf(sv, entry.getBits());
            paritycount += sf.getParityModifiedCount();

            sfbuf.addSubframe(sf);

            if (sfbuf.isEphemerisComplete())
            {
                bnav::SubframeBufferParam data = sfbuf.flushEphemerisData();
                (void) data; // avoid warnings
                ++ephcount;
            }

            else if (sfbuf.isAlmanacComplete())
            {
                bnav::SubframeBufferParam data = sfbuf.flushAlmanacData();
                (void) data; // avoid warnings
                ++almcount;
            }

            ++msgcount;
        }
        CHECK(msgcount == 750);
        // we should have completed 30 ephemeris data sets
        CHECK(ephcount == 14);
        // and 1 almanac data set
        CHECK(almcount == 1);
        CHECK(paritycount == 0);
        reader.close();
    }

    // data has one complete set of frame 1 (page 1 to 10), but at TOW 361284
    // (sbf) there is a parity error, so that there is an offset of 256
    // with that error uncorrected there would be no complete ephemeris data set
    // check if error gets corrected accordingly
    TEST(testSubframeBuffer_ParityFixD2)
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/subframebuffer/sow-parity.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        bnav::SubframeBufferD2 sfbuf;

        std::size_t msgcount = 0, ephcount = 0, paritycount = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            CHECK(sv.isGeo());
            bnav::Subframe sf(sv, entry.getBits());
            paritycount += sf.getParityModifiedCount();

            sfbuf.addSubframe(sf);

            if (sfbuf.isEphemerisComplete())
            {
                bnav::SubframeBufferParam data = sfbuf.flushEphemerisData();
                (void) data; // avoid warnings
                ++ephcount;
            }

            ++msgcount;
        }
        CHECK(msgcount == 52);
        // we should have on complete data set
        CHECK(ephcount == 1);
        // one parity should be fixed!
        CHECK(paritycount == 1);
        reader.close();
    }
}
