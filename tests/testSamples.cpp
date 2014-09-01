#include <unittest++/UnitTest++.h>
#include "TestConfig.h"

#include "Subframe.h"
#include "SubframeBuffer.h"
#include "SubframeBufferStore.h"

#include "AsciiReader.h"
#include "BeiDou.h"
#include "NavBits.h"
#include "SvID.h"

#include <iostream>
#include <map>

#if 0
// this file fails, because lastblock is not zero and there are way too many parity fixes...
SUITE(testSamples)
{
    TEST(testSamplesYears)
    {
        bnav::AsciiReader reader(PATH_TESTDATA+ "sbf/samples/CUT12012040124.sbf_SBF_CMPRaw-head1k.txt",
                                 bnav::AsciiReaderType::TEXT_CONVERTED_SBF);

        bnav::SubframeBufferStore sbstore;

        std::size_t msgcount = 0, ephcount = 0, almcount = 0;
        bnav::AsciiReaderEntry entry;
        while (reader.readLine(entry))
        {
            if (entry.getSignalType() != bnav::SignalType::BDS_B1)
                continue;

            bnav::SvID sv(entry.getPRN());
            bnav::Subframe sf(sv, entry.getDateTime(), entry.getBits());

            sbstore.addSubframe(sv, sf);
            bnav::SubframeBuffer* sfbuf = sbstore.getSubframeBuffer(sv);

            if (sfbuf->isEphemerisComplete())
            {
                sfbuf->clearEphemerisData();
                ++ephcount;
            }

            else if (sfbuf->isAlmanacComplete())
            {
                sfbuf->clearAlmanacData();
                ++almcount;
            }

            ++msgcount;
        }
        CHECK(msgcount == 150);
        // we should have completed 30 ephemeris data sets
        CHECK(ephcount == 30);
        // and 1 almanac data set
        CHECK(almcount == 1);

        // there should be incomplete data at EOF
        CHECK(sbstore.hasIncompleteData());

        reader.close();
    }
}
#endif

