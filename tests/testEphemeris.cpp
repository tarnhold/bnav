#include <UnitTest++/UnitTest++.h>
#include "TestConfig.h"

#include "BeiDou.h"
#include "Ephemeris.h"

#include <cstdint>
#include <iostream>

TEST(testKlobucharParam)
{
    // default constructor
    {
        bnav::KlobucharParam klob;
        CHECK(klob.rawbits.to_ulong() == 0);
        CHECK_CLOSE(0.0, klob.alpha0, 0.0001);
        CHECK_CLOSE(0.0, klob.alpha1, 0.0001);
        CHECK_CLOSE(0.0, klob.alpha2, 0.0001);
        CHECK_CLOSE(0.0, klob.alpha3, 0.0001);
        CHECK_CLOSE(0.0, klob.beta0, 0.0001);
        CHECK_CLOSE(0.0, klob.beta1, 0.0001);
        CHECK_CLOSE(0.0, klob.beta2, 0.0001);
        CHECK_CLOSE(0.0, klob.beta3, 0.0001);
    }
    // operator-
    {
        bnav::KlobucharParam klob;
        klob.alpha0 = 1.0;
        klob.alpha1 = 1.0;
        klob.beta0 = -1.0;
        klob.beta1 = -1.0;
        CHECK(klob.rawbits.to_ulong() == 0);
        CHECK_CLOSE(1.0, klob.alpha0, 0.0001);
        CHECK_CLOSE(1.0, klob.alpha1, 0.0001);
        CHECK_CLOSE(0.0, klob.alpha2, 0.0001);
        CHECK_CLOSE(0.0, klob.alpha3, 0.0001);
        CHECK_CLOSE(-1.0, klob.beta0, 0.0001);
        CHECK_CLOSE(-1.0, klob.beta1, 0.0001);
        CHECK_CLOSE(0.0, klob.beta2, 0.0001);
        CHECK_CLOSE(0.0, klob.beta3, 0.0001);

        bnav::KlobucharParam klob2;
        klob2.rawbits = bnav::NavBits<64>(666);
        klob2.alpha1 = 1.0;
        klob2.alpha2 = 1.0;
        klob2.beta1 = 1.0;
        klob2.beta2 = 1.0;

        // operator- is absolute difference
        bnav::KlobucharParam klobdiff = klob - klob2;
        CHECK(klobdiff.rawbits.to_ulong() == 0);
        CHECK_CLOSE(1.0, klobdiff.alpha0, 0.0001);
        CHECK_CLOSE(0.0, klobdiff.alpha1, 0.0001);
        CHECK_CLOSE(1.0, klobdiff.alpha2, 0.0001);
        CHECK_CLOSE(0.0, klobdiff.alpha3, 0.0001);
        CHECK_CLOSE(1.0, klobdiff.beta0, 0.0001);
        CHECK_CLOSE(2.0, klobdiff.beta1, 0.0001);
        CHECK_CLOSE(1.0, klobdiff.beta2, 0.0001);
        CHECK_CLOSE(0.0, klobdiff.beta3, 0.0001);
    }
    // operator== and operator!=
    {
        bnav::KlobucharParam klob;
        klob.rawbits = bnav::NavBits<64>(UINT32_MAX);
        klob.alpha0 = 99.9;
        CHECK(klob.rawbits.to_ulong() == UINT32_MAX);
        CHECK_CLOSE(99.9, klob.alpha0, 0.0001);

        bnav::KlobucharParam klob2;
        klob2.rawbits = bnav::NavBits<64>(666);
        klob2.alpha1 = 1.0;

        CHECK(!(klob == klob2));
        CHECK(klob != klob2);

        // only the 32 lsb bits are set, that's unlikely
        klob.rawbits = bnav::NavBits<64>(666);
        CHECK(!(klob == klob2));
        CHECK(klob != klob2);

        // now both 32 bit parts have a value
        klob.rawbits.flipLeft(0);
        klob2.rawbits.flipLeft(0);
        CHECK(klob == klob2);
        CHECK(!(klob != klob2));
    }
}
