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
            CHECK(entry.getPRN() == prnlist[i]);
            CHECK(entry.getDateTime() == bnav::DateTime(bnav::TimeSystem::GPST, 1801, towlist[i] / 1000, towlist[i] % 1000));

            // TODO: write data into file and compare to already converted file
            // TestWriter.write(), TestFile.isEqual(filename)...
            //std::cout << entry.getPRN() << " " << entry.getTOW() << " " << entry.getBits() << std::endl;
            ++i;
        }
        // ensure we have all elements
        CHECK(i == 20);

        // FIXME: check at least bits of the last entry, until we use a proper compare file
        CHECK(entry.getBits().to_string() == "111000100100000010010101000011010111110111001000111101010101111111111110000000000011110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111100000111111111100001110111011111011010011100010111111111011100100100");

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

            CHECK(entry.getPRN() == prnlist[i]);
            CHECK(entry.getDateTime() == bnav::DateTime(bnav::TimeSystem::GPST, 1801, towlist[i] / 1000, towlist[i] % 1000));

            // TODO: write data into file and compare to already converted file
            // TestWriter.write(), TestFile.isEqual(filename)...
            //std::cout << entry.getPRN() << " " << entry.getTOW() << " " << entry.getBits() << std::endl;
            ++i;
        }
        // ensure we have all elements
        CHECK(i == 11);

        // FIXME: check at least bits of the last entry, until we use a proper compare file
        CHECK(entry.getBits().to_string() == "111000100100000010010101000011010111110111001000111101010101111111111110000000000011110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111100000111111111100001110111011111011010011100010111111111011100100100");

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
            CHECK(entry.getPRN() == 2);
            CHECK(entry.getDateTime() == bnav::DateTime(bnav::TimeSystem::GPST, 0, towlist[i]));

            // TODO: write data into file and compare to already converted file
            // TestWriter.write(), TestFile.isEqual(filename)...
            //std::cout << entry.getTOW() << " " << entry.getTOW() << " " << entry.getBits() << std::endl;
            ++i;
        }
        // ensure we have all elements
        CHECK(i == 37);

        // FIXME: check at least bits of the last entry, until we use a proper compare file
        CHECK(entry.getBits().to_string() == "111000100100000100010010110000101110110101011101110101111110110111011101110111011101101010011101110111011110000011010101000000001111000000000010011001000111100000000000000101100011010101010101010101010101001011010101010101010101010101001011010101010101010101010101001011010101010101010101010101001011");

        reader.close();
        CHECK(!reader.isOpen());
    }
}
