#include <UnitTest++/UnitTest++.h>
#include "TestConfig.h"

#include "Subframe.h"

#include "AsciiReader.h"
#include "BeiDou.h"
#include "NavBits.h"
#include "SvID.h"

#include <iostream>

// Test real data
SUITE(testSubframe_SBF_Simple)
{
    TEST(testSubframe_SimpleD1)
    {
        bnav::AsciiReader reader(PATH_TESTDATA + "sbf/subframe/prn6-fraID.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        constexpr uint32_t sowlist[] = {345600, 345606, 345612, 345618, 345624};

        std::size_t i = 0, paritycount = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            CHECK(!sv.isGeo());
            bnav::Subframe sf(sv, entry.getBits());
            paritycount += sf.getParityModifiedCount();

            CHECK_EQUAL(i + 1, sf.getFrameID());

            // non-Geo: FraID 1-3 have no Pnum
            if (sf.getFrameID() < 4)
                CHECK_EQUAL(0, sf.getPageNum());
            else
                CHECK_EQUAL(1, sf.getPageNum());

            CHECK_EQUAL(sowlist[i], sf.getSOW());

            ++i;
        }
        CHECK_EQUAL(5, i);
        CHECK_EQUAL(0, paritycount);
        reader.close();
    }

    // same as testSubframeFraIDSimpleD1, but with empty constructor
    TEST(testSubframe_SimpleD1_empty)
    {
        bnav::AsciiReader reader(PATH_TESTDATA + "sbf/subframe/prn6-fraID.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        constexpr uint32_t sowlist[] = {345600, 345606, 345612, 345618, 345624};

        std::size_t i = 0, paritycount = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            CHECK(!sv.isGeo());

            // only difference
            bnav::Subframe sf;
            sf.setBits(entry.getBits());
            sf.setSvID(sv);
            sf.initialize();
            paritycount += sf.getParityModifiedCount();

            CHECK_EQUAL(i + 1, sf.getFrameID());

            // non-Geo: FraID 1-3 have no Pnum
            if (sf.getFrameID() < 4)
                CHECK_EQUAL(0, sf.getPageNum());
            else
                CHECK_EQUAL(1, sf.getPageNum());

            CHECK_EQUAL(sowlist[i], sf.getSOW());

            ++i;
        }
        CHECK_EQUAL(5, i);
        CHECK_EQUAL(0, paritycount);
        reader.close();
    }

    TEST(testSubframe_SimpleD2)
    {
        bnav::AsciiReader reader(PATH_TESTDATA + "sbf/subframe/prn2-fraID.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        std::size_t i = 0, paritycount = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            CHECK(sv.isGeo());
            bnav::Subframe sf(sv, entry.getBits());
            paritycount += sf.getParityModifiedCount();

            CHECK_EQUAL(i + 1, sf.getFrameID());

            // Geo: FraID 3 and 4 have no Pnum
            if (sf.getFrameID() == 3 || sf.getFrameID() == 4)
                CHECK_EQUAL(0, sf.getPageNum());
            else
                CHECK_EQUAL(1, sf.getPageNum());

            // Geo: have same SOW for FraID 1-5
            CHECK_EQUAL(345600, sf.getSOW());

            ++i;
        }
        CHECK_EQUAL(5, i);
        CHECK_EQUAL(0, paritycount);
        reader.close();
    }
}

SUITE(testSubframe_SBF_OneFrame)
{
    TEST(testSubframe_OneFrameD1)
    {
        bnav::AsciiReader reader(PATH_TESTDATA + "sbf/subframe/prn6-oneframe.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        constexpr uint32_t sowfirst = 346320;

        std::size_t i = 0, paritycount = 0;
        std::size_t pnum_fra4 = 0;
        std::size_t pnum_fra5 = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            CHECK(!sv.isGeo());
            bnav::Subframe sf(sv, entry.getBits());
            paritycount += sf.getParityModifiedCount();

            CHECK(sf.getFrameID() == (i % 5) + 1);

            // check Pnum for frame 4 and 5
            if (sf.getFrameID() == 4)
            {
                ++pnum_fra4;
                CHECK_EQUAL(pnum_fra4, sf.getPageNum());
            }
            else if (sf.getFrameID() == 5)
            {
                ++pnum_fra5;
                CHECK_EQUAL(pnum_fra5, sf.getPageNum());
            }
            else
                // non-Geo: FraID 1-3 have no Pnum
                CHECK_EQUAL(0, sf.getPageNum());

            // subframes come every 6s
            CHECK_EQUAL(sowfirst + i * 6, sf.getSOW());

            ++i;
        }
        CHECK_EQUAL(24 * 5, i);
        CHECK_EQUAL(24, pnum_fra4);
        CHECK_EQUAL(24, pnum_fra5);
        CHECK_EQUAL(0, paritycount);
        reader.close();
    }

    TEST(testSubframe_OneFrameD2)
    {
        bnav::AsciiReader reader(PATH_TESTDATA + "sbf/subframe/prn2-oneframe.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        // set first sow
        uint32_t sow = 365040;

        std::size_t i = 0, paritycount = 0;
        std::size_t pnum_fra1 = 0;
        std::size_t pnum_fra2 = 0;
        std::size_t pnum_fra5 = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            CHECK(sv.isGeo());
            bnav::Subframe sf(sv, entry.getBits());
            paritycount += sf.getParityModifiedCount();

            CHECK_EQUAL((i % 5) + 1, sf.getFrameID());
            CHECK_EQUAL(sow, sf.getSOW());

            // check Pnum for frame 1, 2 and 5
            if (sf.getFrameID() == 1)
            {
                // reset after 10 pages
                if (pnum_fra1 == 10)
                    pnum_fra1 = 0;
                ++pnum_fra1;
                CHECK_EQUAL(pnum_fra1, sf.getPageNum());
            }
            else if (sf.getFrameID() == 2)
            {
                // reset after 6 pages
                if (pnum_fra2 == 6)
                    pnum_fra2 = 0;
                ++pnum_fra2;
                CHECK_EQUAL(pnum_fra2, sf.getPageNum());
            }
            else if (sf.getFrameID() == 5)
            {
                ++pnum_fra5;
                CHECK_EQUAL(pnum_fra5, sf.getPageNum());

                // increment SOW if we hit frame 5
                // with D2 we have subframes every 0.6s which is 0.6s * 5 = 3s
                // BeiDou transmits a new SOW on every FraID == 1 for D2
                sow += 3;
            }
            else
                // Geo: FraID 3 and 4 have no Pnum
                CHECK_EQUAL(0, sf.getPageNum());

            ++i;
        }
        CHECK_EQUAL(120 * 5, i);
        CHECK_EQUAL(10, pnum_fra1);
        CHECK_EQUAL(6, pnum_fra2);
        CHECK_EQUAL(120, pnum_fra5);
        CHECK_EQUAL(365397 + 3, sow); // because we incremented one too much
        CHECK_EQUAL(0, paritycount);
        reader.close();
    }
}
