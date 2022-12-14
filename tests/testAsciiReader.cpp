#include <UnitTest++/UnitTest++.h>
#include "TestConfig.h"

#include "AsciiReader.h"
#include "DateTime.h"

#include "BeiDou.h"

#include <iostream>
#include <sstream>

TEST(testAsciiReaderSimple) {
    std::string filename(PATH_TESTDATA + "sbf/CUT12014071724.sbf_SBF_CMPRaw-snip.txt");

    // default constructor
    {
        bnav::AsciiReader reader;
        CHECK(!reader.isOpen());
        // with a default constructor, we need to set the type manually
        CHECK(reader.getType() == bnav::AsciiReaderType::NONE);
        reader.setType(bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
        CHECK(reader.getType() == bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
        reader.open(filename.c_str());
        CHECK(reader.isOpen());
        reader.close();
        CHECK(!reader.isOpen());
    }

    // "comfort" constructor
    {
        bnav::AsciiReader reader(filename, bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
        // should be opened automatically
        CHECK(reader.isOpen());
        CHECK(reader.getType() == bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
        reader.close();
        CHECK(!reader.isOpen());
    }
}

// process all sbf entries
TEST(testAsciiReaderSBF) {
    std::stringstream ssfile;
    ssfile << PATH_TESTDATA << "sbf/CUT12014071724.sbf_SBF_CMPRaw-snip.txt";

    // read a simple sbf file
    {
        bnav::AsciiReader reader;
        reader.setType(bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
        CHECK(reader.getType() == bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
        reader.open(ssfile.str());
        CHECK(reader.isOpen());

        constexpr uint32_t prnlist[] = {3,3,2,5,1,1,4,2,2,5,5,4,3,3,1,1,4,2,2,5};
        constexpr uint32_t towlist[] = {345600200,345600200,345605000,345605000,
                               345605600,345605600,345605600,345605600,
                               345605600,345605600,345605600,345605600,
                               345606200,345606200,345606200,345606200,
                               345606200,345606200,345606200,345606200};

        std::size_t i = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            CHECK_EQUAL(prnlist[i], entry.getPRN());
            CHECK(entry.getDateTime() == bnav::DateTime(bnav::TimeSystem::GPST, 1801, towlist[i] / 1000, towlist[i] % 1000));

            // TODO: write data into file and compare to already converted file
            // TestWriter.write(), TestFile.isEqual(filename)...
            //std::cout << entry.getPRN() << " " << entry.getTOW() << " " << entry.getBits() << std::endl;
            ++i;
        }
        // ensure we have all elements
        CHECK_EQUAL(20, i);

        // FIXME: check at least bits of the last entry, until we use a proper compare file
        CHECK_EQUAL("1110001001000000100101010000110101111101110010001111010101"
                    "0111111111111000000000001111000000000000000000000000000000"
                    "0000000000000000000000000000000000000000000000000000000000"
                    "0000000000000000000000000000000000000000000000000000011111"
                    "0000011111111110000111011101111101101001110001011111111101"
                    "1100100100", entry.getBits().to_string());

        reader.close();
        CHECK(!reader.isOpen());
    }
}

// process B1 sbf entries. Newer RxTools give hex values, not decimals.
TEST(testAsciiReaderSBFHexB1Prn2) {
    std::stringstream ssfile;
    ssfile << PATH_TESTDATA << "sbf.hex/CUT12016122024.sbf_SBF_CMPRaw-snip500-prn2-hex.txt";

    // read a simple sbf file
    {
        bnav::AsciiReader reader;
        reader.setType(bnav::AsciiReaderType::TEXT_CONVERTED_SBF_HEX);
        CHECK(reader.getType() == bnav::AsciiReaderType::TEXT_CONVERTED_SBF_HEX);
        reader.open(ssfile.str());
        CHECK(reader.isOpen());

        const uint32_t tow_first = 172800200;
        const uint32_t tow_last = 172949600;
        uint32_t cur_tow = 0;

        std::uint32_t i = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            cur_tow = tow_first + i * 600;

            CHECK_EQUAL(entry.getPRN(), 2);
            CHECK_EQUAL(bnav::DateTime(bnav::TimeSystem::GPST, 1928, cur_tow / 1000, cur_tow % 1000).getISODate(), entry.getDateTime().getISODate());

            // TODO: write data into file and compare to already converted file
            // TestWriter.write(), TestFile.isEqual(filename)...
            ++i;
        }
        // ensure we have all elements
        CHECK_EQUAL(250, i);
        CHECK_EQUAL(tow_last, cur_tow);

        // FIXME: check at least bits of the last entry, until we use a proper compare file
        CHECK_EQUAL("1110001001000000010010101010000011100001110110010111001101"
                    "0011100001000010111100100000110110111010101101000001000101"
                    "1010101100000000000101010110001101010101010101010101010101"
                    "0010110101010101010101010101010010110101010101010101010101"
                    "0100101101010101010101010101010100101101010101010101010101"
                    "0101001011", entry.getBits().to_string());

        reader.close();
        CHECK(!reader.isOpen());
    }
}

// process only B1 signal messages
TEST(testAsciiReaderSBFB1) {
    std::stringstream ssfile;
    ssfile << PATH_TESTDATA << "sbf/CUT12014071724.sbf_SBF_CMPRaw-snip.txt";

    // read a simple sbf file
    {
        bnav::AsciiReader reader;
        reader.setType(bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
        CHECK(reader.getType() == bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
        reader.open(ssfile.str());
        CHECK(reader.isOpen());

        // we process only B1 entries
        constexpr uint32_t prnlist[] = {3,5,1,4,2,5,3,1,4,2,5};
        constexpr uint32_t towlist[] = {345600200,345605000,345605600,345605600,
                               345605600,345605600,345606200,345606200,
                               345606200,345606200,345606200};

        std::size_t i = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            CHECK_EQUAL(prnlist[i], entry.getPRN());
            CHECK(entry.getDateTime() == bnav::DateTime(bnav::TimeSystem::GPST, 1801, towlist[i] / 1000, towlist[i] % 1000));

            // TODO: write data into file and compare to already converted file
            // TestWriter.write(), TestFile.isEqual(filename)...
            //std::cout << entry.getPRN() << " " << entry.getTOW() << " " << entry.getBits() << std::endl;
            ++i;
        }
        // ensure we have all elements
        CHECK_EQUAL(11, i);

        // FIXME: check at least bits of the last entry, until we use a proper compare file
        CHECK_EQUAL("1110001001000000100101010000110101111101110010001111010101"
                    "0111111111111000000000001111000000000000000000000000000000"
                    "0000000000000000000000000000000000000000000000000000000000"
                    "0000000000000000000000000000000000000000000000000000011111"
                    "0000011111111110000111011101111101101001110001011111111101"
                    "1100100100", entry.getBits().to_string());

        reader.close();
        CHECK(!reader.isOpen());
    }
}

TEST(TestAsciiReaderJPS) {
    std::stringstream ssfile;
    ssfile << PATH_TESTDATA << "jps/821_all_raw_eph-snip.txt";

    // read a simple jps file
    {
        bnav::AsciiReader reader;
        reader.setType(bnav::AsciiReaderType::TEXT_CONVERTED_JPS);
        CHECK(reader.getType() == bnav::AsciiReaderType::TEXT_CONVERTED_JPS);
        reader.open(ssfile.str());
        CHECK(reader.isOpen());

        constexpr uint32_t towlist[] = {310190,310190,310191,310192,310192,310193,
                               310193,310195,310195,310196,310197,310198,
                               310198,310199,310199,310201,310201,310202,
                               310202,310203,310203,310204,310205,310205,
                               310206,310207,310207,310208,310208,310209,
                               310210,310210,310211,310211,310212,310213,
                               310213};
        std::size_t i = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            // this file has only prn 2 data
            CHECK_EQUAL(2, entry.getPRN());
            CHECK(entry.getDateTime() == bnav::DateTime(bnav::TimeSystem::GPST, 0, towlist[i]));

            // TODO: write data into file and compare to already converted file
            // TestWriter.write(), TestFile.isEqual(filename)...
            //std::cout << entry.getTOW() << " " << entry.getTOW() << " " << entry.getBits() << std::endl;
            ++i;
        }
        // ensure we have all elements
        CHECK_EQUAL(37, i);

        // FIXME: check at least bits of the last entry, until we use a proper compare file
        CHECK_EQUAL("1110001001000001000100101100001011101101010111011101011111"
                    "1011011101110111011101110110101001110111011101111000001101"
                    "0101000000001111000000000010011001000111100000000000000101"
                    "1000110101010101010101010101010010110101010101010101010101"
                    "0100101101010101010101010101010100101101010101010101010101"
                    "0101001011", entry.getBits().to_string());

        reader.close();
        CHECK(!reader.isOpen());
    }
}
