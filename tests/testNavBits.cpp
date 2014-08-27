#include <unittest++/UnitTest++.h>

#include "AsciiReader.h"
#include "NavBits.h"

#include <iostream>

TEST(testNavBitsConstructor)
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

        // fill leading pos with zeros
        const bnav::NavBits<5> bits5(31);
        const bnav::NavBits<10> bits6(bits5);
        CHECK(bits6.size() == 10);
        CHECK(bits6.to_string() == "0000011111");
    }

    // string constructor
    {
        // we reserve 10 bits, but only set the 6 lsb bits
        const bnav::NavBits<10> bits3("101000");
        CHECK(bits3.size() == 10);
        CHECK(bits3.to_string() == "0000101000");

        const bnav::NavBits<5> bits4("10100");
        CHECK(bits4.size() == 5);
        CHECK(bits4.to_string() == "10100");
    }

    // bitset constructor, same size
    {
        const std::bitset<5> bitset4(31);
        const bnav::NavBits<10> bits5(bitset4);
        CHECK(bits5.size() == 10);
        CHECK(bits5.to_string() == "0000011111");
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

    // NavBits constructor, same size
    {
        const bnav::NavBits<10> bits1("1111100001");
        const bnav::NavBits<10> bits2(bits1);
        CHECK(bits2.size() == 10);
        CHECK(bits2.to_string() == "1111100001");

        const bnav::NavBits<10> bits3(1023);
        const bnav::NavBits<10> bits4(bits3);
        CHECK(bits4.size() == 10);
        CHECK(bits4.to_string() == "1111111111");
    }

    // NavBits constructor
    {
        // init with smaller NavBits dim
        const bnav::NavBits<5> bits7("11111");
        const bnav::NavBits<10> bits8(bits7);
        CHECK(bits8.size() == 10);
        CHECK(bits8.to_string() == "0000011111");

        // init with empty NavBits
        const bnav::NavBits<5> bits9;
        const bnav::NavBits<10> bits10(bits9);
        CHECK(bits10.size() == 10);
        CHECK(bits10.to_ulong() == 0);
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
        bits.setLeft(0, true);
        CHECK(bits.size() == 3);
        CHECK(bits.atLeft(0) == true);
        CHECK(bits.atLeft(1) == false);
        CHECK(bits.atLeft(2) == false);
        CHECK(bits.to_string() == "100");

        bnav::NavBits<3> bits2;
        bits2.setLeft(0, false);
        bits2.setLeft(1, true);
        bits2.setLeft(2);
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

    // flip all
    {
        {
            bnav::NavBits<4> bits("0001");
            bits.flipAll();
            CHECK(bits.to_string() == "1110");
            bits.flipAll();
            CHECK(bits.to_string() == "0001");
        }
        {
            bnav::NavBits<4> bits("1010");
            bits.flipAll();
            CHECK(bits.to_string() == "0101");
            bits.flipAll();
            CHECK(bits.to_string() == "1010");
        }
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

        // differently sized
        bnav::NavBits<10> bits3;
        bnav::NavBits<5> bits4(31);
        bits3 = bits3 ^ bits4;
        CHECK(bits3.to_string() == "0000011111");
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

        // differently sized
        bnav::NavBits<10> bits3;
        bnav::NavBits<5> bits4(31);
        bits3 ^= bits4;
        CHECK(bits3.to_string() == "0000011111");
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

TEST(testNavBitsEquality)
{
    // Equality, operator==
    {
        bnav::NavBits<4> bits1("0001");
        bnav::NavBits<4> bits2("0001");
        CHECK(bits1 == bits2);

        bnav::NavBits<4> bits3("1000");
        CHECK(!(bits1 == bits3));
    }
}

SUITE(testNavBitsToDouble)
{
    // Two's complement, no scale, positive values
    TEST(testNavBitsToDouble_Positive)
    {
        bnav::NavBits<8> bits1("00000000");
        CHECK_CLOSE(0.0, bits1.to_double(), 0.0);

        bnav::NavBits<8> bits2("00000001");
        CHECK_CLOSE(1.0, bits2.to_double(), 0.0);

        bnav::NavBits<8> bits3("00000010");
        CHECK_CLOSE(2.0, bits3.to_double(), 0.0);

        bnav::NavBits<8> bits4("01111110");
        CHECK_CLOSE(126.0, bits4.to_double(), 0.0);

        bnav::NavBits<8> bits5("01111111");
        CHECK_CLOSE(127.0, bits5.to_double(), 0.0);
    }

    // Two's complement, no scale, negative values
    TEST(testNavBitsToDouble_Negative)
    {
        bnav::NavBits<8> bits1("11111111");
        CHECK_CLOSE(-1.0, bits1.to_double(), 0.0);

        bnav::NavBits<8> bits2("11111110");
        CHECK_CLOSE(-2.0, bits2.to_double(), 0.0);

        bnav::NavBits<8> bits4("10000010");
        CHECK_CLOSE(-126.0, bits4.to_double(), 0.0);

        bnav::NavBits<8> bits5("10000001");
        CHECK_CLOSE(-127.0, bits5.to_double(), 0.0);

        bnav::NavBits<8> bits3("10000000");
        CHECK_CLOSE(-128.0, bits3.to_double(), 0.0);
    }

    // Two's complement, with scale, positive values
    TEST(testNavBitsToDouble_PositiveScale)
    {
        bnav::NavBits<8> bits1("00000000");
        CHECK_CLOSE(0.0, bits1.to_double(2), 0.0);

        bnav::NavBits<8> bits2("00000001");
        CHECK_CLOSE(4.0, bits2.to_double(2), 0.0);

        bnav::NavBits<8> bits3("00000010");
        CHECK_CLOSE(8.0, bits3.to_double(2), 0.0);
        // some huge scales
        CHECK_CLOSE(2048.0, bits3.to_double(10), 0.0);
        CHECK_CLOSE(2097152.0, bits3.to_double(20), 0.0);
        CHECK_CLOSE(2147483648.0, bits3.to_double(30), 0.0);
        CHECK_CLOSE(2199023255552.0, bits3.to_double(40), 0.0);

        bnav::NavBits<8> bits4("01111110");
        CHECK_CLOSE(504.0, bits4.to_double(2), 0.0);

        bnav::NavBits<8> bits5("01111111");
        CHECK_CLOSE(508.0, bits5.to_double(2), 0.0);
    }

    // Two's complement, with scale, negative values
    TEST(testNavBitsToDouble_NegativeScale)
    {
        bnav::NavBits<8> bits1("11111111");
        CHECK_CLOSE(-0.25, bits1.to_double(-2), 0.0);
        // some huge scales
        CHECK_CLOSE(-0.0009765625, bits1.to_double(-10), 1.0e-10);
        CHECK_CLOSE(-0.00000095367431640625, bits1.to_double(-20), 1.0e-20);
        CHECK_CLOSE(-0.000000000931322574615478515625, bits1.to_double(-30), 1.0e-30);

        bnav::NavBits<8> bits2("11111110");
        CHECK_CLOSE(-0.5, bits2.to_double(-2), 0.0);

        bnav::NavBits<8> bits4("10000010");
        CHECK_CLOSE(-31.5, bits4.to_double(-2), 0.0);

        bnav::NavBits<8> bits5("10000001");
        CHECK_CLOSE(-31.75, bits5.to_double(-2), 0.0);

        bnav::NavBits<8> bits3("10000000");
        CHECK_CLOSE(-32.0, bits3.to_double(-2), 0.0);
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
