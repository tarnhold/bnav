#include <UnitTest++/UnitTest++.h>
#include "TestConfig.h"

#include "Subframe.h"
#include "SubframeBuffer.h"
#include "SubframeBufferStore.h"

#include "AsciiReader.h"
#include "BeiDou.h"
#include "NavBits.h"
#include "SvID.h"

#include <iostream>
#include <map>

// Test real data
// Only for data sets from one single PRN
SUITE(testSubframeBufferStore_SBF_Superframe_OnePRN)
{
    TEST(testSubframeBufferStore_SuperframeD1)
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/subframebuffer/CUT12014071724.sbf_SBF_CMPRaw-prn6-onesuperframe.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        bnav::SubframeBufferStore sbstore;

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

            sbstore.addSubframe(sv, sf);
            bnav::SubframeBuffer* sfbuf = sbstore.getSubframeBuffer(sv);

            if (sfbuf->isEphemerisComplete())
            {
                sfbuf->clearEphemerisData();
                ++ephcount;
            }

            else if (sfbuf->isAlmanacComplete())
            {
                sfbuf->clearAlmanacData();
                ++almcount;
            }

            ++msgcount;
        }
        CHECK_EQUAL(150, msgcount);
        // we should have completed 30 ephemeris data sets
        CHECK_EQUAL(30, ephcount);
        // and 1 almanac data set
        CHECK_EQUAL(1, almcount);
        // all parities should be fine
        CHECK_EQUAL(0, paritycount);

        // there should be incomplete data at EOF
        CHECK(sbstore.hasIncompleteData());

        reader.close();
    }

    TEST(testSubframe_SuperframeD2)
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/subframebuffer/CUT12014071724.sbf_SBF_CMPRaw-prn2-onesuperframe.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        bnav::SubframeBufferStore sbstore;

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

            sbstore.addSubframe(sv, sf);
            bnav::SubframeBuffer* sfbuf = sbstore.getSubframeBuffer(sv);

            if (sfbuf->isEphemerisComplete())
            {
                bnav::SubframeBufferParam data = sfbuf->flushEphemerisData();
                (void) data; // avoid warnings
                ++ephcount;
            }

            else if (sfbuf->isAlmanacComplete())
            {
                bnav::SubframeBufferParam data = sfbuf->flushAlmanacData();
                (void) data; // avoid warnings
                ++almcount;
            }

            ++msgcount;
        }
        CHECK_EQUAL(750, msgcount);
        // we should have completed 14 ephemeris data sets
        CHECK_EQUAL(14, ephcount);
        // and 1 almanac data set
        CHECK_EQUAL(1, almcount);
        CHECK_EQUAL(0, paritycount);

        // there should be incomplete data at EOF
        CHECK(sbstore.hasIncompleteData());
        reader.close();
    }
}

// complete data set, with all PRNs
SUITE(testSubframeBufferStore_SBF)
{
    TEST(testSubframeBufferStore15k)
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/subframebuffer/CUT12014071724.sbf_SBF_CMPRaw-snip15k.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        bnav::SubframeBufferStore sbstore;

        std::size_t msgcount = 0, paritycount = 0;
        std::map<bnav::SvID, std::size_t> ephcount;
        std::map<bnav::SvID, std::size_t> almcount;

        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            bnav::Subframe sf(sv, entry.getBits());
            paritycount += sf.getParityModifiedCount();

            sbstore.addSubframe(sv, sf);
            bnav::SubframeBuffer* sfbuf = sbstore.getSubframeBuffer(sv);

            if (sfbuf->isEphemerisComplete())
            {
                sfbuf->clearEphemerisData();
                ++ephcount[sv];
            }

            else if (sfbuf->isAlmanacComplete())
            {
                sfbuf->clearAlmanacData();
                ++almcount[sv];
            }

            ++msgcount;
        }
        CHECK_EQUAL(7500, msgcount);
        CHECK_EQUAL(0, paritycount);

        // we should have ephemeris data from 5 geos and 3 non-geos
        CHECK_EQUAL(8, ephcount.size());
        for (auto it = ephcount.begin(); it != ephcount.end(); ++it)
            CHECK_EQUAL(28, it->second);

        // we should only have a two complete data sets for GEOs
        // because non-GEOs should not be completed
        for (auto it = almcount.cbegin(); it != almcount.cend(); ++it)
        {
            CHECK(it->first.isGeo());
            CHECK_EQUAL(2, it->second);
        }

        // there should be incomplete data at EOF
        CHECK(sbstore.hasIncompleteData());

        reader.close();
    }

    TEST(testSubframeBufferStore20k)
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/subframebuffer/CUT12014071324.sbf_SBF_CMPRaw-snip20k.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        bnav::SubframeBufferStore sbstore;

        std::size_t msgcount = 0, paritycount = 0;
        std::map<bnav::SvID, std::size_t> ephcount;
        std::map<bnav::SvID, std::size_t> almcount;

        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            bnav::Subframe sf(sv, entry.getBits());
            paritycount += sf.getParityModifiedCount();

            sbstore.addSubframe(sv, sf);
            bnav::SubframeBuffer* sfbuf = sbstore.getSubframeBuffer(sv);

            if (sfbuf->isEphemerisComplete())
            {
                sfbuf->clearEphemerisData();
                ++ephcount[sv];
            }

            else if (sfbuf->isAlmanacComplete())
            {
                sfbuf->clearAlmanacData();
                ++almcount[sv];
            }

            ++msgcount;
        }
        CHECK_EQUAL(10000, msgcount);
        CHECK_EQUAL(0, paritycount);

        // we should have ephemeris data from 5 geos and 3 non-geos
        CHECK_EQUAL(8, ephcount.size());
        for (auto it = ephcount.begin(); it != ephcount.end(); ++it)
        {
            CHECK_EQUAL(37, it->second);
        }

        // we should only have a three complete data sets for GEOs
        // and one completed for non-GEOs
        for (auto it = almcount.cbegin(); it != almcount.cend(); ++it)
        {
            if (it->first.isGeo())
                CHECK_EQUAL(3, it->second);
            else
                CHECK_EQUAL(1, it->second);
        }

        // there should be incomplete data at EOF
        CHECK(sbstore.hasIncompleteData());

        reader.close();
    }
}
