#include <unittest++/UnitTest++.h>
#include "TestConfig.h"

#include "Subframe.h"

#include "AsciiReader.h"
#include "BDSCommon.h"
#include "NavBits.h"
#include "SvID.h"

#include <iostream>

TEST(testSubframeConstructor)
{
    // empty constructor
    {
        bnav::NavBits<300> bits;
        //bnav::Subframe sf;
        //CHECK(bits.size() == 10);
        //CHECK(bits.to_string() == "0000000000");
    }

    // non-geo
    {
        // create empty message with preamble
        bnav::NavBits<11> pre(bnav::BDS_PREABMLE);
        bnav::NavBits<300> bits;
        bits ^= pre;
        bits <<= (300 - 11);

    /*    bnav::Subframe sf(0, bits);
        CHECK(sf.getFrameID() == 0);
        CHECK(sf.getPageNum() == 0);
        CHECK(sf.getSOW() == 0);
        CHECK(sf.getTOW() == 0);*/
    }

    // non-geo
    {
        // create empty message with preamble and fraID
        bnav::NavBits<11> pre(bnav::BDS_PREABMLE);
        bnav::NavBits<300> bits;
        bits ^= pre;
        //std::cout << bits << std::endl;
        bits <<= (4 + 3); // shift Rev and FraID

        for (std::size_t i = 1; i <= 5; ++i)
        {
            const bnav::NavBits<3> fraID(i);
            bits ^= fraID; // insert FraID
 //           std::cout << bits << std::endl;
            bits ^= fraID; // unset FraID
        }


/*
        bnav::Subframe sf(0, bits);
        CHECK(sf.getFrameID() == 0);
        CHECK(sf.getPageNum() == 0);
        CHECK(sf.getSOW() == 0);
        CHECK(sf.getTOW() == 0);
        */
    }



    // geo
    {
        bnav::NavBits<300> bits;
       // bnav::Subframe sf(0, bits, true);
    }
}

TEST(testSubframeFraIDSimpleD1)
{
    std::stringstream ssfile;
    ssfile << PATH_TESTDATA << "sbf/prn6-fraID.txt";

    bnav::AsciiReader reader;
    reader.setType(bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
    CHECK(reader.getType() == bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
    reader.open(ssfile.str());
    CHECK(reader.isOpen());

    const uint32_t sowlist[] = {345600, 345606, 345612, 345618, 345624};

    std::size_t i = 0;
    bnav::ReaderNavEntry entry;
    while (reader.readLine(entry))
    {
        if (entry.getSignalType() != bnav::SignalType::BDS_B1)
            continue;

        bnav::SvID sv(entry.getPRN());
        CHECK(!sv.isGeo());
        bnav::Subframe sf(entry.getTOW(), entry.getBits(), sv.isGeo());

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
}

TEST(testSubframeFraIDSimpleD2)
{
    std::stringstream ssfile;
    ssfile << PATH_TESTDATA << "sbf/prn2-fraID.txt";

    bnav::AsciiReader reader;
    reader.setType(bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
    CHECK(reader.getType() == bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
    reader.open(ssfile.str());
    CHECK(reader.isOpen());

    std::size_t i = 0;
    bnav::ReaderNavEntry entry;
    while (reader.readLine(entry))
    {
        if (entry.getSignalType() != bnav::SignalType::BDS_B1)
            continue;

        bnav::SvID sv(entry.getPRN());
        CHECK(sv.isGeo());
        bnav::Subframe sf(entry.getTOW(), entry.getBits(), sv.isGeo());

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
}

TEST(testSubframeFraIDOneFrameD1)
{
    std::stringstream ssfile;
    ssfile << PATH_TESTDATA << "sbf/prn6-oneframe.txt";

    bnav::AsciiReader reader;
    reader.setType(bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
    CHECK(reader.getType() == bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
    reader.open(ssfile.str());
    CHECK(reader.isOpen());

    const uint32_t sowfirst = 346320;

    std::size_t i = 0;
    std::size_t pnum_fra4 = 0;
    std::size_t pnum_fra5 = 0;
    bnav::ReaderNavEntry entry;
    while (reader.readLine(entry))
    {
        if (entry.getSignalType() != bnav::SignalType::BDS_B1)
            continue;

        bnav::SvID sv(entry.getPRN());
        CHECK(!sv.isGeo());
        bnav::Subframe sf(entry.getTOW(), entry.getBits(), sv.isGeo());

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
}

TEST(testSubframeFraIDOneFrameD2)
{
    std::stringstream ssfile;
    ssfile << PATH_TESTDATA << "sbf/prn2-oneframe.txt";

    bnav::AsciiReader reader;
    reader.setType(bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
    CHECK(reader.getType() == bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
    reader.open(ssfile.str());
    CHECK(reader.isOpen());

    // set first sow
    uint32_t sow = 365040;

    std::size_t i = 0;
    std::size_t pnum_fra1 = 0;
    std::size_t pnum_fra2 = 0;
    std::size_t pnum_fra5 = 0;
    bnav::ReaderNavEntry entry;
    while (reader.readLine(entry))
    {
        if (entry.getSignalType() != bnav::SignalType::BDS_B1)
            continue;

        bnav::SvID sv(entry.getPRN());
        CHECK(sv.isGeo());
        bnav::Subframe sf(entry.getTOW(), entry.getBits(), sv.isGeo());

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
}
