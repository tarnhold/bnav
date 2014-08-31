#include <unittest++/UnitTest++.h>
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

// FIXME: check ecc counts!

// Test real data
// Only for data sets from one single PRN
SUITE(testSubframeBufferStore_SBF_Superframe_OnePRN)
{
    TEST(testSubframeBufferStore_SuperframeD1)
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/subframebuffer/CUT12014071724.sbf_SBF_CMPRaw-prn6-onesuperframe.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        bnav::SubframeBufferStore sbstore;

        std::size_t msgcount = 0, ephcount = 0, almcount = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            CHECK(!sv.isGeo());
            bnav::Subframe sf(sv, entry.getDateTime(), entry.getBits());

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
        CHECK(msgcount == 150);
        // we should have completed 30 ephemeris data sets
        CHECK(ephcount == 30);
        // and 1 almanac data set
        CHECK(almcount == 1);

        // there should be incomplete data at EOF
        CHECK(sbstore.hasIncompleteData());

        reader.close();
    }

    TEST(testSubframe_SuperframeD2)
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/subframebuffer/CUT12014071724.sbf_SBF_CMPRaw-prn2-onesuperframe.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        bnav::SubframeBufferStore sbstore;

        std::size_t msgcount = 0, ephcount = 0, almcount = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            CHECK(sv.isGeo());
            bnav::Subframe sf(sv, entry.getDateTime(), entry.getBits());

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
        CHECK(msgcount == 750);
        // we should have completed 30 ephemeris data sets
        CHECK(ephcount == 14);
        // and 1 almanac data set
        CHECK(almcount == 1);

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

        std::size_t msgcount = 0;
        std::map<bnav::SvID, std::size_t> ephcount;
        std::map<bnav::SvID, std::size_t> almcount;

        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            bnav::Subframe sf(sv, entry.getDateTime(), entry.getBits());

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
        CHECK(msgcount == 7500);

        // we should have ephemeris data from 5 geos and 3 non-geos
        CHECK(ephcount.size() == 8);
        for (auto it = ephcount.begin(); it != ephcount.end(); ++it)
            CHECK(it->second == 28);

        // we should only have a two complete data sets for GEOs
        // because non-GEOs should not be completed
        for (auto it = almcount.cbegin(); it != almcount.cend(); ++it)
        {
            CHECK(it->first.isGeo());
            CHECK(it->second == 2);
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

        std::size_t msgcount = 0;
        std::map<bnav::SvID, std::size_t> ephcount;
        std::map<bnav::SvID, std::size_t> almcount;

        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            bnav::Subframe sf(sv, entry.getDateTime(), entry.getBits());

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
        CHECK(msgcount == 10000);

        // we should have ephemeris data from 5 geos and 3 non-geos
        CHECK(ephcount.size() == 8);
        for (auto it = ephcount.begin(); it != ephcount.end(); ++it)
        {
            CHECK(it->second == 37);
        }

        // we should only have a three complete data sets for GEOs
        // and one completed for non-GEOs
        for (auto it = almcount.cbegin(); it != almcount.cend(); ++it)
        {
            if (it->first.isGeo())
                CHECK(it->second == 3);
            else
                CHECK(it->second == 1);
        }

        // there should be incomplete data at EOF
        CHECK(sbstore.hasIncompleteData());

        reader.close();
    }
}
