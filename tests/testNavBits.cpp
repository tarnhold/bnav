#include <unittest++/UnitTest++.h>

#include "AsciiReader.h"
#include "NavBits.h"

#include <iostream>

TEST(testNavBitsConstructor)
{
    // Constructor tests
    {
        // empty constructor
        {
            const bnav::NavBits<10> bits;
            CHECK(bits.size() == 10);
            CHECK(bits.to_string() == "0000000000");
        }

        // numeric constructor
        {
            const bnav::NavBits<10> bits2(8);
            CHECK(bits2.size() == 10);
            CHECK(bits2.to_ulong() == 8);

            const bnav::NavBits<10> bits3(1023);
            CHECK(bits3.size() == 10);
            CHECK(bits3.to_string() == "1111111111");
        }

        // string costructor
        {
            // we reserve 10 bits, but only set the 6 lsb bits
            const bnav::NavBits<10> bits3("101000");
            CHECK(bits3.size() == 10);
            CHECK(bits3.to_string() == "0000101000");

            const bnav::NavBits<5> bits4("10100");
            CHECK(bits4.size() == 5);
            CHECK(bits4.to_string() == "10100");
        }

        // bitest constructor
        {
            const std::bitset<10> bitset1("1111100001");
            const bnav::NavBits<10> bits2(bitset1);
            CHECK(bits2.size() == 10);
            CHECK(bits2.to_string() == "1111100001");

            const std::bitset<10> bitset2(1023);
            const bnav::NavBits<10> bits3(bitset2);
            CHECK(bits3.size() == 10);
            CHECK(bits3.to_string() == "1111111111");
        }
    }
}

TEST(testNavBitsAccess)
{
    // Access from right, like std::bitset
    // operator[] and &operator[]
    {
        bnav::NavBits<3> bits;
        bits[0] = true;
        CHECK(bits[0] == true);
        CHECK(bits[1] == false);
        CHECK(bits[2] == false);
        CHECK(bits.to_string() == "001");

        bnav::NavBits<3> bits2;
        bits2[1] = true;
        bits2[2] = true;
        CHECK(bits2[0] == false);
        CHECK(bits2[1] == true);
        CHECK(bits2[2] == true);
        CHECK(bits2.to_string() == "110");
    }

    // Access from left
    // atLeft and &atLeft
    {
        bnav::NavBits<3> bits;
        bits.atLeft(0) = true;
        CHECK(bits.size() == 3);
        CHECK(bits.atLeft(0) == true);
        CHECK(bits.atLeft(1) == false);
        CHECK(bits.atLeft(2) == false);
        CHECK(bits.to_string() == "100");

        bnav::NavBits<3> bits2;
        bits2.atLeft(1) = true;
        bits2.atLeft(2) = true;
        CHECK(bits2.atLeft(0) == false);
        CHECK(bits2.atLeft(1) == true);
        CHECK(bits2.atLeft(2) == true);
        CHECK(bits2.to_string() == "011");
    }
}

TEST(testNavBitsFlip)
{
    // flip single bits
    {
        bnav::NavBits<4> bits("0001");
        bits.flip(0);
        CHECK(bits.to_string() == "0000");
        bits.flip(2);
        CHECK(bits.to_string() == "0100");
    }

    // flip from left
    {
        bnav::NavBits<4> bits("0001");
        bits.flipLeft(0);
        CHECK(bits.to_string() == "1001");
        bits.flipLeft(1);
        CHECK(bits.to_string() == "1101");
    }
}

TEST(testNavBitsXor)
{
    // operator^
    {
        bnav::NavBits<4> bits("0000");
        const bnav::NavBits<4> bits2("1100");
        bits = bits ^ bits2;
        CHECK(bits.to_string() == "1100");
        bits.flip(0);
        CHECK(bits.to_string() == "1101");
        bits = bits ^ bits2;
        CHECK(bits.to_string() == "0001");
    }

    // operator^=
    {
        bnav::NavBits<4> bits("0000");
        const bnav::NavBits<4> bits2("1100");
        bits ^= bits2;
        CHECK(bits.to_string() == "1100");
        bits.flip(0);
        CHECK(bits.to_string() == "1101");
        bits ^= bits2;
        CHECK(bits.to_string() == "0001");
    }
}

TEST(testNavBitsShift)
{
    // Left bitshift, operator<<=
    {
        bnav::NavBits<4> bits("0001");
        bits <<= 3;
        CHECK(bits.to_string() == "1000");
        bits <<= 1;
        CHECK(bits.to_string() == "0000");
    }
}

TEST(testNavBitsSlice)
{
    // Get bit slice from left (msb is index 0)
    {
        const bnav::NavBits<6> bTest("111001");

        std::string ret = bTest.getLeft<0, 6>().to_string();
        CHECK(ret == "111001");
        ret = bTest.getLeft<1, 5>().to_string();
        CHECK(ret == "11001");
        ret = bTest.getLeft<2, 4>().to_string();
        CHECK(ret == "1001");
        ret = bTest.getLeft<3, 3>().to_string();
        CHECK(ret == "001");
        ret = bTest.getLeft<4, 2>().to_string();
        CHECK(ret == "01");
        ret = bTest.getLeft<5, 1>().to_string();
        CHECK(ret == "1");
        ret = bTest.getLeft<6, 0>().to_string();
        CHECK(ret == "");
    }
}
