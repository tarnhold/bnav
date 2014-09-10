#include "AsciiReader.h"
#include "BeiDou.h"
#include "Ephemeris.h"
#include "IonexWriter.h"
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
    bnav::IonosphereStore ionostoreKlobuchar;

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

    uint32_t twoHourCountOld = UINT32_MAX;
    bnav::Ionosphere iono_old;
    bnav::KlobucharParam klob_old;

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
        std::cout << "prn: " << data.getPRN() << " wn: " << data.getDateTime().getWeekNum()
                  << " tow: " << data.getDateTime().getSOW() << " sow: " << sf.getSOW()
                  << " fra: " << sf.getFrameID() << " pnum: "
                  << sf.getPageNum() << std::endl;
#endif

        sbstore.addSubframe(sv, sf);

        bnav::SubframeBuffer* sfbuf = sbstore.getSubframeBuffer(sv);

        if (sfbuf->isEphemerisComplete())
        {
            bnav::SubframeBufferParam bdata = sfbuf->flushEphemerisData();
            //std::cout << "eph complete" << std::endl;

            bnav::Ephemeris eph(bdata);

            // Model is updated at every full two hour (00:00, 02:00, 04:00,...).
            // Try to get at least one model within this time frame. It may
            // be the case, that there is no data until 01:50, but with this
            // we can grep the model within the last 10 minutes of transmission.
            uint32_t twoHourCount = eph.getSOW() / 7200;
            if (twoHourCount != twoHourCountOld)
            {
                twoHourCountOld = twoHourCount;
                bnav::KlobucharParam klob = eph.getKlobucharParam();

                // Take only one new model.
                if (klob != klob_old)
                {
                    std::cout << "New Klobuchar Model at SOW: " << eph.getSOW() << std::endl;

                    // If we get a model at 01:50 we need to correct the SOW down to
                    // 00:00, because this was the date of issue for this model.
                    // We calculate the Klobuchar model only on every change of the
                    // model parameters (every two hours). Otherwise the model
                    // slightly changes with each new SOW, because it's dependent
                    // on the local time.
                    uint32_t secondOfTwoHours = eph.getSOW() % 7200;
                    uint32_t sowFullTwoHour = eph.getSOW() - secondOfTwoHours;
                    bnav::Ionosphere ionoklob(klob, sowFullTwoHour);

                    //std::cout << klob << std::endl;

                    ionoklob.dump();
                    ionostoreKlobuchar.addIonosphere(sv, ionoklob);

                    klob_old = klob;
                }
            }
#if 0
            ephstore.add(sv, eph);
#endif
        }
        else if (sfbuf->isAlmanacComplete())
        {
            bnav::SubframeBufferParam bdata = sfbuf->flushAlmanacData();
            //std::cout << "almanac complete" << std::endl;

            bnav::Ionosphere iono(bdata);

//#if 0
            // diff only for one single prn
            if (sv.getPRN() == 2)
            {
//                if (iono_old.hasData())
//                    iono.diffToModel(iono_old).dump();

                iono.dump();
                iono_old = iono;
            }
//#endif

            //bnav::Ionosphere ionoclone(bdata);
            //std::cout << (ionoclone == iono) << std::endl;

//            ionostore.addIonosphere(sv, iono);
#if 0
            bnav::Almanac alm(data);

#endif
        }
    }
    reader.close();

#if 0
    std::string ionexfilename = argv[2];
    //ionexfilename = /// aus YYYYMMDD o.ae. zusammensetzen
    bnav::IonexWriter writer(filename, true);
    if (!writer.isOpen())
        std::perror(("Error: Could not open file: " + filename).c_str());

    writer.writerHeader();
    for (auto it = ionostore.begin(); it != ionostore.end(); ++it)
        writer.writeData(it);
    writer.close();
#endif

    if (sbstore.hasIncompleteData())
        std::cout << "SubframeBufferStore has incomplete data sets at EOF. Ignoring." << std::endl;

    return 0;
}

