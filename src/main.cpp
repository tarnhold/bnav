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

#include <cstdlib>

static void usage()
{
    std::cout
            << "\n"
            << "usage: bapp [OPTION] [FILENAME]\n\n"
            << "\t-jps\tRead file as converted Javad file\n"
            << "\t-sbf\tRead file as converted Septentrio file\n"
            << std::endl;
}

/**
 * @brief lcl_extractDateTimeFromFilename Extract date and time from file name.
 * @param filename Filename string in IGS format.
 * @return DateTime object.
 */
bnav::DateTime lcl_extractDateFromFilename(const std::string &filename)
{
    std::string filename_date;
    const std::size_t lastslash = filename.find_last_of('/');

    if (lastslash != std::string::npos)
        // dir/filename.ext
        filename_date = filename.substr(lastslash + 5, 8);
    else
        // filename.ext
        filename_date = filename.substr(4, 8);

    // add time to get a ISO date YYYYMMDDTHHMMSS
    filename_date += "T000000";

    return bnav::DateTime(bnav::TimeSystem::BDT, filename_date);
}

/**
 * @brief lcl_extractStationFromFilename Extract station name from file name.
 * @param filename Filename string in IGS format.
 * @return Station name as string.
 */
std::string lcl_extractStationFromFilename(const std::string &filename)
{
    std::string station;
    const std::size_t lastslash = filename.find_last_of('/');

    if (lastslash != std::string::npos)
        // dir/filename.ext
        station = filename.substr(lastslash + 1, 4);
    else
        // filename.ext
        station = filename.substr(0, 4);

    return station;
}

int main(int argc, char **argv)
{
    // require at least two parameters
    if (argc != 3)
    {
        usage();
        return EXIT_FAILURE;
    }

    // parse first argument
    const std::string arg = argv[1];
    bnav::AsciiReaderType filetype(bnav::AsciiReaderType::NONE);
    if (arg == "-jps")
        filetype = bnav::AsciiReaderType::TEXT_CONVERTED_JPS;
    else if (arg == "-sbf")
        filetype = bnav::AsciiReaderType::TEXT_CONVERTED_SBF;
    else
    {
        std::cerr << "Warning: Unknown argument: " << arg << std::endl;
        usage();
        return EXIT_FAILURE;
    }

    // Open file and parse lines
    const std::string filename = argv[2];
    bnav::AsciiReader reader(filename, filetype);
    if (!reader.isOpen())
        std::perror(("Error: Could not open file: " + filename).c_str());

    bnav::SubframeBufferStore sbstore;
    bnav::IonosphereStore ionostore;
    bnav::IonosphereStore ionostoreKlobuchar;

#if 0
    bnav::EphemerisStore ephStore;
#endif

    // extract date from filename, so we have a clue which data we want
    // to extract from the file (it's possible that there is more than
    // one day data in the file.
    const bnav::DateTime dtfilename { lcl_extractDateFromFilename(filename) };
    const std::string station { lcl_extractStationFromFilename(filename) };

    uint32_t weeknum {0};
    uint32_t twoHourCountOld = UINT32_MAX;
    bnav::Ionosphere iono_old;
    bnav::KlobucharParam klob_old;

    bnav::AsciiReaderEntry data;
    while (reader.readLine(data))
    {
        // skip B2 signals, the differences are not in our interest
        if (data.getSignalType() != bnav::SignalType::BDS_B1)
            continue;

        const bnav::SvID sv(data.getPRN());

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
            const bnav::SubframeBufferParam bdata = sfbuf->flushEphemerisData();
            //std::cout << "eph complete" << std::endl;

            bnav::Ephemeris eph(bdata);
            // store weeknum, because it's only present in Ephemeris data, we
            // need this for Ionosphere, too.
            weeknum = eph.getWeekNum();

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
                    bnav::DateTime ephdate { bnav::TimeSystem::BDT, weeknum, sowFullTwoHour };
                    bnav::Ionosphere ionoklob(klob, ephdate);

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
            const bnav::SubframeBufferParam bdata = sfbuf->flushAlmanacData();
            //std::cout << "almanac complete" << std::endl;

            if (weeknum != 0)
            {

            bnav::Ionosphere iono(bdata, weeknum);

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

            ionostore.addIonosphere(sv, iono);
#if 0
            bnav::Almanac alm(data);

#endif
            }
        }
    }
    reader.close();


//#if 0
    // overwrites without warnings
    std::string ionexfilename = station + dtfilename.getISODate() + ".inx";
    std::cout << ionexfilename << std::endl;
    bnav::IonexWriter writer(ionexfilename, true);
    if (!writer.isOpen())
        std::perror(("Error: Could not open file: " + filename).c_str());

    // write all models from prn2
    const auto prn2data = ionostore.getItemsBySv(bnav::SvID(2));
    writer.writeAll(prn2data);
    writer.close();
//#endif

    if (sbstore.hasIncompleteData())
        std::cout << "SubframeBufferStore has incomplete data sets at EOF. Ignoring." << std::endl;

    return EXIT_SUCCESS;
}

