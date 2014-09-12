#include <unittest++/UnitTest++.h>
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
            bnav::Subframe sf(sv, entry.getDateTime(), entry.getBits());
            paritycount += sf.getParityModifiedCount();

            CHECK(sf.getFrameID() == i + 1);

            // non-Geo: FraID 1-3 have no Pnum
            if (sf.getFrameID() < 4)
                CHECK(sf.getPageNum() == 0);
            else
                CHECK(sf.getPageNum() == 1);

            CHECK(sf.getSOW() == sowlist[i]);

            ++i;
        }
        CHECK(i == 5);
        CHECK(paritycount == 0);
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
            sf.setDateTime(entry.getDateTime());
            sf.setBits(entry.getBits());
            sf.setSvID(sv);
            sf.initialize();
            paritycount += sf.getParityModifiedCount();

            CHECK(sf.getFrameID() == i + 1);

            // non-Geo: FraID 1-3 have no Pnum
            if (sf.getFrameID() < 4)
                CHECK(sf.getPageNum() == 0);
            else
                CHECK(sf.getPageNum() == 1);

            CHECK(sf.getSOW() == sowlist[i]);

            ++i;
        }
        CHECK(i == 5);
        CHECK(paritycount == 0);
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
            bnav::Subframe sf(sv, entry.getDateTime(), entry.getBits());
            paritycount += sf.getParityModifiedCount();

            CHECK(sf.getFrameID() == i + 1);

            // Geo: FraID 3 and 4 have no Pnum
            if (sf.getFrameID() == 3 || sf.getFrameID() == 4)
                CHECK(sf.getPageNum() == 0);
            else
                CHECK(sf.getPageNum() == 1);

            // Geo: have same SOW for FraID 1-5
            CHECK(sf.getSOW() == 345600);

            ++i;
        }
        CHECK(i == 5);
        CHECK(paritycount == 0);
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
            bnav::Subframe sf(sv, entry.getDateTime(), entry.getBits());
            paritycount += sf.getParityModifiedCount();

            CHECK(sf.getFrameID() == (i % 5) + 1);

            // check Pnum for frame 4 and 5
            if (sf.getFrameID() == 4)
            {
                ++pnum_fra4;
                CHECK(sf.getPageNum() == pnum_fra4);
            }
            else if (sf.getFrameID() == 5)
            {
                ++pnum_fra5;
                CHECK(sf.getPageNum() == pnum_fra5);
            }
            else
                // non-Geo: FraID 1-3 have no Pnum
                CHECK(sf.getPageNum() == 0);

            // subframes come every 6s
            CHECK(sf.getSOW() == sowfirst + i * 6);

            ++i;
        }
        CHECK(i == 24*5);
        CHECK(pnum_fra4 == 24);
        CHECK(pnum_fra5 == 24);
        CHECK(paritycount == 0);
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
            bnav::Subframe sf(sv, entry.getDateTime(), entry.getBits());
            paritycount += sf.getParityModifiedCount();

            CHECK(sf.getFrameID() == (i % 5) + 1);
            CHECK(sf.getSOW() == sow);

            // check Pnum for frame 1, 2 and 5
            if (sf.getFrameID() == 1)
            {
                // reset after 10 pages
                if (pnum_fra1 == 10)
                    pnum_fra1 = 0;
                ++pnum_fra1;
                CHECK(sf.getPageNum() == pnum_fra1);
            }
            else if (sf.getFrameID() == 2)
            {
                // reset after 6 pages
                if (pnum_fra2 == 6)
                    pnum_fra2 = 0;
                ++pnum_fra2;
                CHECK(sf.getPageNum() == pnum_fra2);
            }
            else if (sf.getFrameID() == 5)
            {
                ++pnum_fra5;
                CHECK(sf.getPageNum() == pnum_fra5);

                // increment SOW if we hit frame 5
                // with D2 we have subframes every 0.6s which is 0.6s * 5 = 3s
                // BeiDou transmits a new SOW on every FraID == 1 for D2
                sow += 3;
            }
            else
                // Geo: FraID 3 and 4 have no Pnum
                CHECK(sf.getPageNum() == 0);

            ++i;
        }
        CHECK(i == 120*5);
        CHECK(pnum_fra1 == 10);
        CHECK(pnum_fra2 == 6);
        CHECK(pnum_fra5 == 120);
        CHECK(sow == 365397 + 3); // because we incremented one too much
        CHECK(paritycount == 0);
        reader.close();
    }
}
