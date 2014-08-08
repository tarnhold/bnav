#include "AsciiReader.h"
#include "BeiDou.h"
#include "Subframe.h"
#include "SubframeBuffer.h"
#include "SvID.h"

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

    bnav::SubframeBuffer sfbuf;
#if 0
    bnav::IonosphereStore ionoStore;
    bnav::EphemerisStore ephStore;
#endif

    bnav::AsciiReaderEntry data;
    while (reader.readLine(data))
    {
        // skip B2 signals, the differences are not in our interest
        if (data.getSignalType() != bnav::SignalType::BDS_B1)
            continue;

        bnav::SvID sv(data.getPRN());
        bnav::Subframe sf(data.getTOW(), data.getBits(), sv.isGeo());

        // debug
        std::cout << "prn: " << data.getPRN()
                  << " tow: " << data.getTOW() << " sow: " << sf.getSOW()
                  << " fra: " << sf.getFrameID() << " pnum: "
                  << sf.getPageNum() << std::endl;

        sfbuf.addSubframe(sf);

        if (sfbuf.isEphemerisComplete())
        {
            bnav::SubframeVector data = sfbuf.flushEphemerisData();
            std::cout << "eph complete" << std::endl;

#if 0
            bnav::SubframeVector data = sfbuf.getAlmanacData();

            bnav::Ephemeris eph(data);
#endif
#if 0
            ephstore.add(eph);
#endif
        }
        else if (sfbuf.isIntegrityComplete())
        {
            sfbuf.clearIntegrityData();
            std::cout << "integrity complete" << std::endl;
        }
        else if (sfbuf.isAlmanacComplete())
        {
            sfbuf.clearAlmanacData();
            std::cout << "almanac complete" << std::endl;

#if 0
            bnav::SubframeVector data = sfbuf.getAlmanacData();

            bnav::Ionosphere iono(data);
            bnav::Almanac alm(data);
#endif
#if 0
            ionoStore.add(iono);
#endif
        }
/*        else if (sfbuf.isIonosphereComplete())
        {
        // iono steckt im almanach drin
            std::cout << "iono complete" << std::endl;
        }
*/
    }
    reader.close();


    return 0;
}

