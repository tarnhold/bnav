#include <unittest++/UnitTest++.h>

#include "NavBits.h"
#include "NavBitsECC.h"

#include <bitset> //temp!
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
#endif

TEST(testNavBitsECC)
{
    //std::bitset<30> bTest("100110010010011101111101010111");
    std::bitset<30> bTest(  "101111111110000000000011110000");

#if 0
    std::bitset<30> bTestM = mirror(bTest);
    std::cout << "normal:" << bTest << std::endl;
    std::cout << "mirror:" << bTestM << " :" << bTestM[29 - 4] << std::endl;
#endif

    std::bitset<90> bTest2("111111111110000000000011111111111000000000001111111111100000000000111100001111000011110000");

    std::bitset<150> bTest3("111111111110000000000011111111111000000000001111111111100000000000111111111110000000000011111111111000000000001111000011110000111100001111000011110000");

    //checkAndFixParity(bTest);

    std::cout << "-------------2" << std::endl;

//    checkAndFixParity(bTest2);

    std::cout << "-------------3" << std::endl;


    bnav::NavBitsECC<30> ecc(bTest);
    ecc.checkAndFixAll();
    std::cerr << ecc.isModified() << std::endl;
    //ecc.checkParity();




    std::cout << "-------------4" << std::endl;

}

TEST(testNavBitsECCParity15)
{
    // set an arbitrary bit message, which parity check would succeed
    std::bitset<15> bits("110110010010101");
    const unsigned long initialbitval = bits.to_ulong();

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
