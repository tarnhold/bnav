#include "bnavMain.h"

#include "BeiDou.h"
#include "Ephemeris.h"
#include "IonexWriter.h"
#include "Ionosphere.h"
#include "Subframe.h"
#include "SubframeBuffer.h"
#include "SvID.h"

#include "DateTime.h"

#include <cstdlib>

#include <boost/program_options.hpp>

namespace
{

#if 0
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
#endif

}

namespace bnav
{

bnavMain::bnavMain(int argc, char *argv[])
    : filenameInput()
    , filetypeInput(bnav::AsciiReaderType::NONE)
    , filenameIonexKlobuchar()
    , filenameIonexRegional()
    , generateGlobalKlobuchar(false)
    , limit_to_interval(0)
    , limit_to_prn(UINT32_MAX)
    , sbstore()
    , ionostore()
    , ionostoreKlobuchar()
{
    boost::program_options::options_description desc("Generic options");
    desc.add_options()
            ("help,h", "show help message")
            ("verbose,v", "verbose output")
            ("format,f", boost::program_options::value<std::string>()->default_value("sbf"), "input file format (sbf or jps)")
            ("klobuchar,k", boost::program_options::value<std::string>(&filenameIonexKlobuchar), "save Klobuchar models to file")
            ("regional,r", boost::program_options::value<std::string>(&filenameIonexRegional), "save regional grid models to file")
            ("global", "generate global Klobuchar model")
            ("sv,s", boost::program_options::value< std::vector<std::size_t> >(), "proceed only specified PRN")
            ("interval,i", boost::program_options::value<std::size_t>(&limit_to_interval)->default_value(7200), "decimate Ionex output to interval [s]")
            ("file", boost::program_options::value<std::string>(&filenameInput)->required(), "input file name");

    boost::program_options::positional_options_description positionalopts;
    positionalopts.add("file", 1);

    boost::program_options::variables_map vm;
    try
    {
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(positionalopts).run(), vm);
        boost::program_options::notify(vm);

        if (vm.count("help"))
        {
            std::stringstream msg;
            msg << desc;
            throw std::runtime_error(msg.str());
        }
        if (vm.count("format"))
        {
            std::string arg = vm["format"].as<std::string>();

            if (arg == "jps")
                filetypeInput = bnav::AsciiReaderType::TEXT_CONVERTED_JPS;
            else if (arg == "sbf")
                filetypeInput = bnav::AsciiReaderType::TEXT_CONVERTED_SBF;
            else
                throw std::invalid_argument("Unknown file format: " + arg);
        }
        if (vm.count("sv"))
        {
            std::vector<std::size_t> svlist = vm["sv"].as< std::vector<std::size_t> >();
            assert(svlist.size() == 1); // atm we can only choose one element
            limit_to_prn = svlist[0];
        }
        if (vm.count("global"))
        {
            // whether to calculate a global Klobuchar model or not
            generateGlobalKlobuchar = true;
        }
        if (vm.count("interval"))
        {
            std::cout << "Setting interval to " << limit_to_interval << "s" << std::endl;
        }
    }
    catch (boost::program_options::too_many_positional_options_error &)
    {
        std::stringstream msg;
        msg << "Error: Only one input file allowed." << std::endl << std::endl << desc;
        throw std::runtime_error(msg.str());
    }
    catch (boost::program_options::error &e)
    {
        std::stringstream msg;
        msg << "Error: " << e.what() << std::endl << std::endl << desc;
        throw std::runtime_error(msg.str());
    }
    catch (std::invalid_argument &e)
    {
        std::stringstream msg;
        msg << "Error: " << e.what() << std::endl;
        throw std::runtime_error(msg.str());
    }
    catch (...)
    {
        std::stringstream msg;
        msg << "Error: Unknown exception!" << std::endl << std::endl << desc;
        throw std::runtime_error(msg.str());
    }
}

void bnavMain::readInputFile()
{
    // Open file and parse lines
    bnav::AsciiReader reader(filenameInput, filetypeInput);
    if (!reader.isOpen())
        std::perror(("Error: Could not open file: " + filenameInput).c_str());

    // extract date from filename, so we have a clue which data we want
    // to extract from the file (it's possible that there is more than
    // one day data in the file.

//FIXME: we need to check if we have IGS filename!
//    if (isIGSFilename(filename))

//std::cout << "bla: " << lcl_extractDateFromFilename(filename).getISODate() << std::endl;
//    const bnav::DateTime dtfilename { lcl_extractDateFromFilename(filename) };

    uint32_t weeknum {0};
    uint32_t intervalCountOld = UINT32_MAX;
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
        //if (!sv.isGeo())
        //    continue;

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
            uint32_t intervalCount = eph.getSOW() / limit_to_interval;
            // FIXME: we take only PRN 2 data here, if we would like to
            // replace missing data of prn 2 with other geos we have to
            // think about IonosphereStore, which stores in depending on SvID!
            if (limit_to_prn != UINT32_MAX && sv.getPRN() == limit_to_prn && intervalCount != intervalCountOld)
            {
                intervalCountOld = intervalCount;
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
                    uint32_t secondOfInterval = eph.getSOW() % limit_to_interval;
                    uint32_t sowFullInterval = eph.getSOW() - secondOfInterval;
                    bnav::DateTime ephdate { bnav::TimeSystem::BDT, weeknum, sowFullInterval };
                    bnav::Ionosphere ionoklob(klob, ephdate, generateGlobalKlobuchar);

                    std::cout << klob << std::endl;
                    ionoklob.dump();
                    std::cout << "add Klobuchar to store for SV: " << sv.getPRN() << std::endl;
                    ionostoreKlobuchar.addIonosphere(sv, ionoklob);

                    klob_old = klob;
                }
            }
        }
        else if (sfbuf->isAlmanacComplete())
        {
            const bnav::SubframeBufferParam bdata = sfbuf->flushAlmanacData();
            //std::cout << "almanac complete" << std::endl;

#if 0
            if (weeknum != 0)
            {

            bnav::Ionosphere iono(bdata, weeknum);

            // diff only for one single prn
            if (limit_to_prn != UINT32_MAX && sv.getPRN() == limit_to_prn && iono.getDateOfIssue().getSOW() % limit_to_interval == 0)
            {
//                if (iono_old.hasData())
//                    iono.diffToModel(iono_old).dump();

                iono.dump();
                ionostore.addIonosphere(sv, iono);

                iono_old = iono;
            }

            //bnav::Ionosphere ionoclone(bdata);
            //std::cout << (ionoclone == iono) << std::endl;



            //bnav::Almanac alm(data);
            }
#endif
        }
    }
    reader.close();

    if (sbstore.hasIncompleteData())
        std::cout << "SubframeBufferStore has incomplete data sets at EOF. Ignoring." << std::endl;

    if (!filenameIonexKlobuchar.empty())
        writeIonexFile(filenameIonexKlobuchar, true);
    if (!filenameIonexRegional.empty())
        writeIonexFile(filenameIonexRegional, false);
}

void bnavMain::writeIonexFile(const std::string &filename, const bool klobuchar)
{
    assert(limit_to_prn < UINT32_MAX); // atm we can only store data from one prn
    // overwrites without warnings
    //std::string ionexfilename = station + dtfilename.getISODate() + ".inx";
    //std::cout << ionexfilename << std::endl;
    bnav::IonexWriter writer(filename, klobuchar);
    if (!writer.isOpen())
        std::perror(("Error: Could not open file: " + filename).c_str());

    // write all models from prn
    const auto prn2data = klobuchar ? ionostoreKlobuchar.getItemsBySv(bnav::SvID(limit_to_prn)) : ionostore.getItemsBySv(bnav::SvID(limit_to_prn));
    writer.writeAll(prn2data);
    writer.close();
}

} // namespace bnav
