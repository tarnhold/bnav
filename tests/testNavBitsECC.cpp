#include <unittest++/UnitTest++.h>

#include "NavBits.h"
#include "NavBitsECC.h"

#include <iostream>

#if 0
/*!
 * Normally a bitset is accessed by indices starting at the least significant
 * bit. To emulate access starting at the most significant bit, we have to
 * mirror all values.
 *
 * Thus we can access the bit at position zero, which is now accessed
 * "from the left".
 *
 */
template <std::size_t len>
std::bitset<len> mirror(const std::bitset<len> &rhs)
{
    std::bitset<len> lhs;

    for (std::size_t i = 0; i < len; ++i)
        lhs[i] = rhs[len - 1 - i];

    return lhs;
}

    std::bitset<30> bTestM = mirror(bTest);
    std::cout << "normal:" << bTest << std::endl;
    std::cout << "mirror:" << bTestM << " :" << bTestM[29 - 4] << std::endl;
#endif

// check various bit blocks, if they got split correctly to "subwords" (11+4 bits)
SUITE(testNavBitsECC_Block)
{
    // ParityBlockType = default
    TEST(testNavBitsECC30)
    {
        bnav::NavBits<30> bTest("111111111110000000000011110000");

        bnav::NavBitsECC<30> ecc(bTest);
        ecc.checkAndFixAll();
        CHECK(!ecc.isModified());

        // change first bit
        bTest.flipLeft(0);
        bnav::NavBitsECC<30> ecc2(bTest);
        ecc2.checkAndFixAll();
        CHECK(ecc2.isModified());
    }

    // ParityBlockType::BITS24
    TEST(testNavBitsECC90)
    {
        bnav::NavBits<90> bTest("111111111110000000000011111111111000000000001111111111100000000000111100001111000011110000");

        bnav::NavBitsECC<90> ecc90(bTest);
        ecc90.checkAndFixAll();
        CHECK(!ecc90.isModified());

        // change first bit of subword 1
        bTest.flipLeft(0);
        bnav::NavBitsECC<90> ecc902(bTest);
        ecc902.checkAndFixAll();
        CHECK(ecc902.isModified());

        // change first bit of subword 2
        bTest.flipLeft(15);
        bnav::NavBitsECC<90> ecc903(bTest);
        ecc903.checkAndFixAll();
        CHECK(ecc903.isModified());
    }

    // ParityBlockType::BITS40
    TEST(testNavBitsECC150)
    {
        bnav::NavBits<150> bTest("111111111110000000000011111111111000000000001111111111100000000000111111111110000000000011111111111000000000001111000011110000111100001111000011110000");

        bnav::NavBitsECC<150> ecc150(bTest);
        ecc150.checkAndFixAll();
        CHECK(!ecc150.isModified());
    }
}

// checks if error correction works for every single bit of one word
TEST(testNavBitsECCParity15)
{
    // set an arbitrary bit message, which parity check would succeed
    bnav::NavBits<15> bits("110110010010101");
    const uint32_t initialbitval = bits.to_ulong();

    bnav::NavBitsECC<15> ecc(bits);
    ecc.checkAndFixAll();
    // ensure nothing was modified, because the bits are ok
    CHECK(!ecc.isModified());
    CHECK(ecc.getBits().to_ulong() == initialbitval);

    // now manipulate every single bit of that message (only one once!)
    // so we can see, that the error correction works for every bit
    for (std::size_t i = 0; i < bits.size(); ++i)
    {
        // flip one bit
        bits.flip(i);

        bnav::NavBitsECC<15> ecc3(bits);
        ecc3.checkAndFixAll();
        CHECK(ecc3.isModified());
        // ensure we recovered the original state
        CHECK(ecc3.getBits().to_ulong() == initialbitval);

        // restore originial state
        bits.flip(i);
        CHECK(bits.to_ulong() == initialbitval);
    }
}
