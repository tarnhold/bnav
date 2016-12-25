#include <UnitTest++/UnitTest++.h>

#include "AsciiReader.h"
#include "NavBits.h"

#include <iostream>

TEST(testNavBitsConstructor)
{
    // empty constructor
    {
        const bnav::NavBits<10> bits;
        CHECK_EQUAL(10, bits.size());
        CHECK_EQUAL("0000000000", bits.to_string());
    }

    // numeric constructor
    {
        const bnav::NavBits<10> bits2(8);
        CHECK_EQUAL(10, bits2.size());
        CHECK_EQUAL(8, bits2.to_ulong());

        const bnav::NavBits<10> bits3(1023);
        CHECK_EQUAL(10, bits3.size());
        CHECK_EQUAL("1111111111", bits3.to_string());

        // fill leading pos with zeros
        const bnav::NavBits<5> bits5(31);
        const bnav::NavBits<10> bits6(bits5);
        CHECK_EQUAL(10, bits6.size());
        CHECK_EQUAL("0000011111", bits6.to_string());
    }

    // string constructor
    {
        // we reserve 10 bits, but only set the 6 lsb bits
        const bnav::NavBits<10> bits3("101000");
        CHECK_EQUAL(10, bits3.size());
        CHECK_EQUAL("0000101000", bits3.to_string());

        const bnav::NavBits<5> bits4("10100");
        CHECK_EQUAL(5, bits4.size());
        CHECK_EQUAL("10100", bits4.to_string());
    }

    // bitset constructor, same size
    {
        const std::bitset<5> bitset4(31);
        const bnav::NavBits<10> bits5(bitset4);
        CHECK_EQUAL(10, bits5.size());
        CHECK_EQUAL("0000011111", bits5.to_string());
    }

    // bitest constructor
    {
        const std::bitset<10> bitset1("1111100001");
        const bnav::NavBits<10> bits2(bitset1);
        CHECK_EQUAL(10, bits2.size());
        CHECK_EQUAL("1111100001", bits2.to_string());

        const std::bitset<10> bitset2(1023);
        const bnav::NavBits<10> bits3(bitset2);
        CHECK_EQUAL(10, bits3.size());
        CHECK_EQUAL("1111111111", bits3.to_string());
    }

    // NavBits constructor, same size
    {
        const bnav::NavBits<10> bits1("1111100001");
        const bnav::NavBits<10> bits2(bits1);
        CHECK_EQUAL(10, bits2.size());
        CHECK_EQUAL("1111100001", bits2.to_string());

        const bnav::NavBits<10> bits3(1023);
        const bnav::NavBits<10> bits4(bits3);
        CHECK_EQUAL(10, bits4.size());
        CHECK_EQUAL("1111111111", bits4.to_string());
    }

    // NavBits constructor
    {
        // init with smaller NavBits dim
        const bnav::NavBits<5> bits7("11111");
        const bnav::NavBits<10> bits8(bits7);
        CHECK_EQUAL(10, bits8.size());
        CHECK_EQUAL("0000011111", bits8.to_string());

        // init with empty NavBits
        const bnav::NavBits<5> bits9;
        const bnav::NavBits<10> bits10(bits9);
        CHECK_EQUAL(10, bits10.size());
        CHECK_EQUAL(0, bits10.to_ulong());
    }
}

SUITE(testNavBitsAccess)
{
    // Access from right, like std::bitset
    // operator[] and &operator[]
    TEST(testNavBitsAccessSimple)
    {
        bnav::NavBits<3> bits;
        bits[0] = true;
        CHECK_EQUAL(true, bits[0]);
        CHECK_EQUAL(false, bits[1]);
        CHECK_EQUAL(false, bits[2]);
        CHECK_EQUAL("001", bits.to_string());

        bnav::NavBits<3> bits2;
        bits2[1] = true;
        bits2[2] = true;
        CHECK_EQUAL(false, bits2[0]);
        CHECK_EQUAL(true, bits2[1]);
        CHECK_EQUAL(true, bits2[2]);
        CHECK_EQUAL("110", bits2.to_string());
    }

    // Access from left
    // atLeft and &atLeft
    TEST(testNavBitsAccessSimpleLeft)
    {
        bnav::NavBits<3> bits;
        bits.setLeft(0, true);
        CHECK_EQUAL(3, bits.size());
        CHECK_EQUAL(true, bits.atLeft(0));
        CHECK_EQUAL(false, bits.atLeft(1));
        CHECK_EQUAL(false, bits.atLeft(2));
        CHECK_EQUAL("100", bits.to_string());

        bnav::NavBits<3> bits2;
        bits2.setLeft(0, false);
        bits2.setLeft(1, true);
        bits2.setLeft(2);
        CHECK_EQUAL(false, bits2.atLeft(0));
        CHECK_EQUAL(true, bits2.atLeft(1));
        CHECK_EQUAL(true, bits2.atLeft(2));
        CHECK_EQUAL("011", bits2.to_string());
    }

    // Access from left, set a bunch of bits
    TEST(testNavBitsAccessSetLeftNavBits)
    {
        const bnav::NavBits<4> setter("1101");
        bnav::NavBits<10> bits;
        bits.setLeft(0, setter);
        CHECK_EQUAL(10, bits.size());
        CHECK_EQUAL("1101000000", bits.to_string());

        bits = bnav::NavBits<10>(0);
        CHECK_EQUAL(0, bits.to_ulong());
        bits.setLeft(1, setter);
        CHECK_EQUAL("0110100000", bits.to_string());

        bits = bnav::NavBits<10>(0);
        CHECK_EQUAL(0, bits.to_ulong());
        bits.setLeft(6, setter);
        CHECK_EQUAL("0000001101", bits.to_string());

        bits = bnav::NavBits<10>(31);
        CHECK_EQUAL("0000011111", bits.to_string());
        bits.setLeft(0, setter);
        CHECK_EQUAL("1101011111", bits.to_string());

        bits = bnav::NavBits<10>(31);
        CHECK_EQUAL("0000011111", bits.to_string());
        bits.setLeft(2, setter);
        CHECK_EQUAL("0011011111", bits.to_string());
    }
}

TEST(testNavBitsFlip)
{
    // flip single bits
    {
        bnav::NavBits<4> bits("0001");
        bits.flip(0);
        CHECK_EQUAL("0000", bits.to_string());
        bits.flip(2);
        CHECK_EQUAL("0100", bits.to_string());
    }

    // flip from left
    {
        bnav::NavBits<4> bits("0001");
        bits.flipLeft(0);
        CHECK_EQUAL("1001", bits.to_string());
        bits.flipLeft(1);
        CHECK_EQUAL("1101", bits.to_string());
    }

    // flip all
    {
        {
            bnav::NavBits<4> bits("0001");
            bits.flipAll();
            CHECK_EQUAL("1110", bits.to_string());
            bits.flipAll();
            CHECK_EQUAL("0001", bits.to_string());
        }
        {
            bnav::NavBits<4> bits("1010");
            bits.flipAll();
            CHECK_EQUAL("0101", bits.to_string());
            bits.flipAll();
            CHECK_EQUAL("1010", bits.to_string());
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
        CHECK_EQUAL("1100", bits.to_string());
        bits.flip(0);
        CHECK_EQUAL("1101", bits.to_string());
        bits = bits ^ bits2;
        CHECK_EQUAL("0001", bits.to_string());

        // differently sized
        bnav::NavBits<10> bits3;
        bnav::NavBits<5> bits4(31);
        bits3 = bits3 ^ bits4;
        CHECK_EQUAL("0000011111", bits3.to_string());
    }

    // operator^=
    {
        bnav::NavBits<4> bits("0000");
        const bnav::NavBits<4> bits2("1100");
        bits ^= bits2;
        CHECK_EQUAL("1100", bits.to_string());
        bits.flip(0);
        CHECK_EQUAL("1101", bits.to_string());
        bits ^= bits2;
        CHECK_EQUAL("0001", bits.to_string());

        // differently sized
        bnav::NavBits<10> bits3;
        bnav::NavBits<5> bits4(31);
        bits3 ^= bits4;
        CHECK_EQUAL("0000011111", bits3.to_string());
    }
}

TEST(testNavBitsShift)
{
    // Left bitshift, operator<<=
    {
        bnav::NavBits<4> bits("0001");
        bits <<= 3;
        CHECK_EQUAL("1000", bits.to_string());
        bits <<= 1;
        CHECK_EQUAL("0000", bits.to_string());
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
        bnav::NavBits<8> bits1("00000000"); // 0
        CHECK_CLOSE(0.0, bits1.to_double(2), 0.0);

        bnav::NavBits<8> bits2("00000001"); // 1
        CHECK_CLOSE(4.0, bits2.to_double(2), 0.0);

        bnav::NavBits<8> bits3("00000010"); // 2
        CHECK_CLOSE(8.0, bits3.to_double(2), 0.0);
        // some huge scales
        CHECK_CLOSE(2048.0, bits3.to_double(10), 0.0);
        CHECK_CLOSE(2097152.0, bits3.to_double(20), 0.0);
        CHECK_CLOSE(2147483648.0, bits3.to_double(30), 0.0);
        CHECK_CLOSE(2199023255552.0, bits3.to_double(40), 0.0);

        bnav::NavBits<8> bits4("01111110"); // 126
        CHECK_CLOSE(504.0, bits4.to_double(2), 0.0);

        bnav::NavBits<8> bits5("01111111"); // 127
        CHECK_CLOSE(508.0, bits5.to_double(2), 0.0);
    }

    // Two's complement, with scale, negative values
    TEST(testNavBitsToDouble_NegativeScale)
    {
        bnav::NavBits<8> bits1("11111111"); // -1
        CHECK_CLOSE(-0.25, bits1.to_double(-2), 0.0);
        // some huge scales
        CHECK_CLOSE(-0.0009765625, bits1.to_double(-10), 1.0e-10);
        CHECK_CLOSE(-0.00000095367431640625, bits1.to_double(-20), 1.0e-20);
        CHECK_CLOSE(-0.000000000931322574615478515625, bits1.to_double(-30), 1.0e-30);

        bnav::NavBits<8> bits2("11111110"); // -2
        CHECK_CLOSE(-0.5, bits2.to_double(-2), 0.0);

        bnav::NavBits<8> bits4("10000010"); // -126
        CHECK_CLOSE(-31.5, bits4.to_double(-2), 0.0);

        bnav::NavBits<8> bits5("10000001"); // -127
        CHECK_CLOSE(-31.75, bits5.to_double(-2), 0.0);

        bnav::NavBits<8> bits3("10000000"); // -128
        CHECK_CLOSE(-32.0, bits3.to_double(-2), 0.0);
    }
}

TEST(testNavBitsSlice)
{
    // Get bit slice from left (msb is index 0)
    {
        const bnav::NavBits<6> bTest("111001");

        std::string ret = bTest.getLeft<0, 6>().to_string();
        CHECK_EQUAL("111001", ret);
        ret = bTest.getLeft<1, 5>().to_string();
        CHECK_EQUAL("11001", ret);
        ret = bTest.getLeft<2, 4>().to_string();
        CHECK_EQUAL("1001", ret);
        ret = bTest.getLeft<3, 3>().to_string();
        CHECK_EQUAL("001", ret);
        ret = bTest.getLeft<4, 2>().to_string();
        CHECK_EQUAL("01", ret);
        ret = bTest.getLeft<5, 1>().to_string();
        CHECK_EQUAL("1", ret);
        ret = bTest.getLeft<6, 0>().to_string();
        CHECK_EQUAL("", ret);
    }

    // Get bit slice from left (msb is index 0), check by assignment
    {
        const bnav::NavBits<6> bTest("111001");

        bnav::NavBits<6> ret6 = bTest.getLeft<0, 6>();
        CHECK_EQUAL("111001", ret6.to_string());
        bnav::NavBits<5> ret5 = bTest.getLeft<1, 5>();
        CHECK_EQUAL("11001", ret5.to_string());
        bnav::NavBits<1> ret1 = bTest.getLeft<5, 1>();
        CHECK_EQUAL("1", ret1.to_string());
        bnav::NavBits<0> ret0 = bTest.getLeft<6, 0>();
        CHECK_EQUAL("", ret0.to_string());
    }
}
