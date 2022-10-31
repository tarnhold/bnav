#include <UnitTest++/UnitTest++.h>
#include "TestConfig.h"

#include "NavBits.h"
#include "NavBitsECC.h"
#include "AsciiReader.h"
#include "SvID.h"
#include "Subframe.h"

#include <iostream>

// check various bit blocks, if they got split correctly to "subwords" (11+4 bits)
SUITE(testNavBitsECC_Block)
{
    // block with 8 parity bits at the end, this is the default word size
    TEST(testNavBitsECC30)
    {
        bnav::NavBits<30> bTest("111111111110000000000011110000");

        bnav::NavBitsECCWord<30> ecc(bTest);
        CHECK(!ecc.isModified());
        // check if bits, which got split into subwords, merge back correctly
        CHECK_EQUAL(bTest, ecc.getBits());

        // change first bit
        bTest.flipLeft(0);
        bnav::NavBitsECCWord<30> ecc2(bTest);
        CHECK(ecc2.isModified());
    }

    // two words, 2x30 bits
    // should split to 30 + 30 -> 11+11+4+4 + 11+11+4+4
    TEST(testNavBitsECC60)
    {
        const bnav::NavBits<60> bTest("111111111110000000000011110000111111111110000000000011110000");

        // we have to check two words, which split into two subwords each
        const bnav::NavBits<30> word1 = bTest.getLeft<0, 30>();
        bnav::NavBitsECCWord<30> ecc(word1);
        CHECK(!ecc.isModified());
        CHECK_EQUAL(word1, ecc.getBits());

        const bnav::NavBits<30> word2 = bTest.getLeft<30, 30>();
        ecc = bnav::NavBitsECCWord<30>(word2);
        CHECK(!ecc.isModified());
        CHECK_EQUAL(word2, ecc.getBits());
    }

    // block with 24 parity bits at the end
    TEST(testNavBitsECC90)
    {
        bnav::NavBits<90> bTest("111111111110000000000011111111111000000000001111111111100000000000111100001111000011110000");

        bnav::NavBitsECCWord<90> ecc90(bTest);
        CHECK(!ecc90.isModified());
        CHECK_EQUAL(bTest, ecc90.getBits());

        // change first bit of subword 1
        bTest.flipLeft(0);
        bnav::NavBitsECCWord<90> ecc902(bTest);
        CHECK(ecc902.isModified());

        // change first bit of subword 2
        bTest.flipLeft(15);
        bnav::NavBitsECCWord<90> ecc903(bTest);
        CHECK(ecc903.isModified());
    }

    // block with 40 parity bits at the end
    TEST(testNavBitsECC150)
    {
        const bnav::NavBits<150> bTest("111111111110000000000011111111111000000000001111111111100000000000111111111110000000000011111111111000000000001111000011110000111100001111000011110000");

        bnav::NavBitsECCWord<150> ecc150(bTest);
        CHECK(!ecc150.isModified());
        CHECK_EQUAL(bTest, ecc150.getBits());
    }

    // block with 72 parity bits at the end
    TEST(testNavBitsECC270)
    {
        const bnav::NavBits<270> bTest("111111111110000000000011111111111000000000001111111111100000000000111111111110000000000011111111111000000000001111111111100000000000111111111110000000000011111111111000000000001111111111100000000000111100001111000011110000111100001111000011110000111100001111000011110000");

        bnav::NavBitsECCWord<270> ecc270(bTest);
        CHECK(!ecc270.isModified());
        // check if bits, which got split into subwords, merge back correctly
        CHECK_EQUAL(bTest, ecc270.getBits());
    }
}

TEST(testNavBitsECC_Examples)
{
    bnav::NavBits<300> bits("111000100100000011010110001011001000110101000100000010011110000010000100111111111000010101001111011101110111011111011010011101110111011101110111011110110111011101110111011101101010011101110111100000000011010100000011110000000000000100110011111000000000000011101110110001101110111011101110111011100110");

    // between bits 30 and 60 should be some parity fix
    bnav::NavBitsECCWord<30> ecc(bits.getLeft<30, 30>());
    CHECK(ecc.isModified());

    ecc = bnav::NavBitsECCWord<30>(bits.getLeft<60, 30>());
    CHECK(!ecc.isModified());
    ecc = bnav::NavBitsECCWord<30>(bits.getLeft<90, 30>());
    CHECK(!ecc.isModified());
    ecc = bnav::NavBitsECCWord<30>(bits.getLeft<120, 30>());
    CHECK(!ecc.isModified());
}

// checks if error correction works for every single bit of one word
TEST(testNavBitsECCParity15)
{
    // set an arbitrary bit message, which parity check would succeed
    bnav::NavBits<15> bits("110110010010101");
    const uint32_t initialbitval = bits.to_uint32_t();

    bnav::NavBitsECCWord<15> ecc(bits);
    // ensure nothing was modified, because the bits are ok
    CHECK(!ecc.isModified());
    CHECK_EQUAL(initialbitval, ecc.getBits().to_uint32_t());

    // now manipulate every single bit of that message (only one once!)
    // so we can see, that the error correction works for every bit
    for (std::size_t i = 0; i < bits.size(); ++i)
    {
        // flip one bit
        bits.flip(i);

        bnav::NavBitsECCWord<15> ecc3(bits);
        CHECK(ecc3.isModified());
        // ensure we recovered the original state
        CHECK_EQUAL(initialbitval, ecc3.getBits().to_uint32_t());

        // restore originial state
        bits.flip(i);
        CHECK_EQUAL(initialbitval, bits.to_uint32_t());
    }
}

TEST(testNavBitsECCWordFiles)
{
    bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/parity/wrong-parities-wordone.txt",
                             bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

    std::size_t msgcount = 0, paritycount = 0;
    bnav::AsciiReaderEntry entry;
    while (reader.readLine(entry))
    {
        if (entry.getSignalType() != bnav::SignalType::BDS_B1)
            continue;

        bnav::SvID sv(entry.getPRN());
        CHECK(sv.isGeo());
        bnav::Subframe sf(sv, entry.getBits());
        paritycount += sf.getParityModifiedCount();

        ++msgcount;
    }
    CHECK_EQUAL(5, msgcount);
    CHECK_EQUAL(1, paritycount);
    reader.close();
}

TEST(testNavBitsECCSamples)
{
    // check B1I and B2I parity counts, from a file, which has many parity fixes
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/parity/CUT12014071324-parities.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        std::size_t msgcount = 0, paritycountB1 = 0, paritycountB2 = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            bnav::SvID sv(entry.getPRN());
            bnav::Subframe sf(sv, entry.getBits());

            // check both, B1I and B2I
            if (entry.getSignalType() == bnav::SignalType::BDS_B1)
                paritycountB1 += sf.getParityModifiedCount();
            else
                paritycountB2 += sf.getParityModifiedCount();

            ++msgcount;
        }
        CHECK_EQUAL(434, msgcount);
        CHECK_EQUAL(11, paritycountB1);
        CHECK_EQUAL(10, paritycountB2);
        reader.close();
    }

    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/parity/CUT12014071724-parities.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        std::size_t msgcount = 0, paritycountB1 = 0, paritycountB2 = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            bnav::SvID sv(entry.getPRN());
            bnav::Subframe sf(sv, entry.getBits());

            // check both, B1I and B2I
            if (entry.getSignalType() == bnav::SignalType::BDS_B1)
                paritycountB1 += sf.getParityModifiedCount();
            else
                paritycountB2 += sf.getParityModifiedCount();

            ++msgcount;
        }
        //std::cout << "msgcount: " << msgcount << " parity: " << paritycountB1 << std::endl;
        CHECK_EQUAL(210, msgcount);
        CHECK_EQUAL(12, paritycountB1);
        CHECK_EQUAL(8, paritycountB2);
        reader.close();
    }
}
