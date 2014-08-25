#include "AsciiReader.h"
#include "BeiDou.h"
#include "Ephemeris.h"
#include "Ionosphere.h"
#include "IonosphereStore.h"
#include "Subframe.h"
#include "SubframeBuffer.h"
#include "SubframeBufferStore.h"
#include "SvID.h"

#include "DateTime.h"

static void usage()
{
    std::cout
            << "\n"
            << "usage: bapp [OPTION] [FILENAME]\n\n"
            << "\t-jps\tRead file as converted Javad file\n"
            << "\t-sbf\tRead file as converted Septentrio file\n"
            << std::endl;
}

int main(int argc, char **argv)
{
    // require at least two parameters
    if (argc != 3)
    {
        usage();
        return 1;
    }

    // parse first argument
    std::string arg = argv[1];
    bnav::AsciiReaderType filetype(bnav::AsciiReaderType::NONE);
    if (arg == "-jps")
        filetype = bnav::AsciiReaderType::TEXT_CONVERTED_JPS;
    else if (arg == "-sbf")
        filetype = bnav::AsciiReaderType::TEXT_CONVERTED_SBF;
    else
    {
        std::cerr << "Warning: Unknown argument: " << arg << std::endl;
        usage();
        return 1;
    }

    // Open file and parse lines
    std::string filename = argv[2];
    bnav::AsciiReader reader(filename, filetype);
    if (!reader.isOpen())
        std::perror(("Error: Could not open file: " + filename).c_str());

    bnav::SubframeBufferStore sbstore;
    bnav::IonosphereStore ionostore;

#if 0
    bnav::EphemerisStore ephStore;
#endif


    bnav::DateTime dt;
//    dt.setToCurrentDateTimeUTC();
    /*
    std::cout << dt.getMonthNameShort() << std::endl;
    std::cout << dt.getIonexDate() << std::endl;

    dt.setWeekAndSOW(0, 0);
    std::cout << dt.getMonthNameShort() << std::endl;
    std::cout << dt.getIonexDate() << std::endl;
*/
    dt.setTimeSystem(bnav::TimeSystem::GPST);
    dt.setWeekAndSOW(1801, 0);
    std::cout << dt.getMonthNameShort() << std::endl;
    std::cout << dt.getIonexDate() << std::endl;

    bnav::Ionosphere iono_old;

    bnav::AsciiReaderEntry data;
    while (reader.readLine(data))
    {
        // skip B2 signals, the differences are not in our interest
        if (data.getSignalType() != bnav::SignalType::BDS_B1)
            continue;

        bnav::SvID sv(data.getPRN());

        // skip non-GEO SVs
        if (!sv.isGeo())
            continue;

        bnav::Subframe sf(sv, data.getDateTime(), data.getBits());

#if 0
        // debug
        std::cout << "prn: " << data.getPRN() << " wn: " << data.getWeek()
                  << " tow: " << data.getTOW() << " sow: " << sf.getSOW()
                  << " fra: " << sf.getFrameID() << " pnum: "
                  << sf.getPageNum() << std::endl;
#endif

        sbstore.addSubframe(sv, sf);

        bnav::SubframeBuffer* sfbuf = sbstore.getSubframeBuffer(sv);

        if (sfbuf->isEphemerisComplete())
        {
            bnav::SubframeBufferParam bdata = sfbuf->flushEphemerisData();
//            std::cout << "eph complete" << std::endl;

//            bnav::Ephemeris eph(bdata);
#if 0
            ephstore.add(sv, eph);
#endif
        }
        else if (sfbuf->isAlmanacComplete())
        {
            bnav::SubframeBufferParam bdata = sfbuf->flushAlmanacData();
            std::cout << "almanac complete" << std::endl;

            bnav::Ionosphere iono(bdata);

            // diff only for one single prn
            if (sv.getPRN() == 2)
            {
                if (iono_old.hasData())
                    iono.diffToModel(iono_old).dump();

                iono_old = iono;
            }

           // iono.dump();

            //bnav::Ionosphere ionoclone(bdata);
            //std::cout << (ionoclone == iono) << std::endl;

            ionostore.addIonosphere(sv, iono);
#if 0
            bnav::Almanac alm(data);

#endif
        }
    }
    reader.close();

    if (sbstore.hasIncompleteData())
        std::cout << "SubframeBufferStore has incomplete data sets at EOF. Ignoring." << std::endl;

    return 0;
}

