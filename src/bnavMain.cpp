#include "bnavMain.h"

#include "BeiDou.h"
#include "Ephemeris.h"
#include "IonexWriter.h"
#include "Ionosphere.h"
#include "Subframe.h"
#include "SubframeBuffer.h"
#include "MessageStatistic.h"

#include "DateTime.h"

#include <cstdlib>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

namespace
{

/**
 * @brief lcl_extractDateStringFromIGSFilename Try to extract date string from
 * IGS filename. If filename isn't in IGS format it just returns an empty string.
 * @param fullfilename Full filename.
 * @return Date as string.
 */
boost::optional<std::string> lcl_extractDateStringFromIGSFilename(const std::string &fullfilename)
{
    boost::optional<std::string> datestring;
    std::string filename = fullfilename;
    const std::size_t lastslash = fullfilename.find_last_of('/');

    // remove dir parts: dir/filename.ext
    if (lastslash != std::string::npos)
        filename = fullfilename.substr(lastslash + 1);

    // CUT12014071324.sbf_SBF_CMPRaw.txt
    boost::regex expr("^[A-Z]{3}[A-Z0-9]([0-9]{8})24\\.");
    // we want only group one
    boost::sregex_token_iterator it(filename.begin(), filename.end(), expr, 1);
    boost::sregex_token_iterator end;

    if (it != end)
        datestring = it->str();

    // if nothing matched, optional is null
    return datestring;
}

}

namespace bnav
{

bnavMain::bnavMain(int argc, char *argv[])
    : filenameInput()
    , filetypeInput(bnav::AsciiReaderType::NONE)
    , filenameIonexKlobuchar()
    , filenameIonexRegional()
    , generateGlobalKlobuchar(false)
    , limit_to_interval_regional(0)
    , limit_to_interval_klobuchar(0)
    , limit_to_prn(boost::optional<SvID>())
    , limit_to_date(boost::optional<DateTime>())
    , sbstore()
    , ionostore()
    , ionostoreKlobuchar()
{
    std::string limit_to_date_str;
    boost::program_options::options_description desc("Generic options");
    desc.add_options()
            ("help,h", "show help message")
            ("verbose,v", "verbose output")
            ("format,f", boost::program_options::value<std::string>()->default_value("sbf"), "input file format (sbf, sbfhex or jps)")
            ("klobuchar,k", boost::program_options::value<std::string>(&filenameIonexKlobuchar), "save Klobuchar models to file")
            ("regional,r", boost::program_options::value<std::string>(&filenameIonexRegional), "save regional grid models to file")
            ("global", "generate global Klobuchar model")
            ("sv,s", boost::program_options::value< std::vector<std::size_t> >(), "proceed only specified PRN")
            ("ir", boost::program_options::value<std::uint32_t>(&limit_to_interval_regional)->default_value(7200), "decimate Regional Ionex output to interval [s]")
            ("ik", boost::program_options::value<std::uint32_t>(&limit_to_interval_klobuchar)->default_value(7200), "decimate Klobuchar Ionex output to interval [s]")
            ("date,d", boost::program_options::value<std::string>(&limit_to_date_str), "limit Ionex output to date")
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
            else if (arg == "sbfhex")
                filetypeInput = bnav::AsciiReaderType::TEXT_CONVERTED_SBF_HEX;
            else
                throw std::invalid_argument("Unknown file format: " + arg);
        }
        if (vm.count("sv"))
        {
            std::vector<std::uint32_t> svlist = vm["sv"].as< std::vector<std::uint32_t> >();
            assert(svlist.size() == 1); // atm we can only choose one element
            limit_to_prn = SvID(svlist[0]);
        }
        if (vm.count("global"))
        {
            // whether to calculate a global Klobuchar model or not
            generateGlobalKlobuchar = true;
        }
        if (vm.count("ir"))
        {
            if (limit_to_interval_regional == 0)
                throw std::invalid_argument("Cannot set interval to zero!");

            // they come all 6 min
            if (limit_to_interval_regional < 360)
                throw std::invalid_argument("Grid data is only available >=360s.");

            std::cout << "Setting interval to " << limit_to_interval_regional << "s" << std::endl;
        }
        if (vm.count("ik"))
        {
            if (limit_to_interval_klobuchar == 0)
                throw std::invalid_argument("Cannot set interval to zero!");

            // With an interval <7200s we would interpolate data. We only
            // want new model data. If there is a need for <7200s data, the
            // klob != klob_old condition has to be removed.
            if (limit_to_interval_klobuchar < 7200)
                throw std::invalid_argument("Interval <7200s is not possible for Klobuchar.");

            std::cout << "Setting interval to " << limit_to_interval_klobuchar << "s" << std::endl;
        }
        if (vm.count("date"))
        {
            // limit data processing to a specific date, this is higher
            // in priority than extracting the date from filename.

            // check for date validity
            if (limit_to_date_str.length() != 8)
                throw std::invalid_argument("Invalid date!");

            //FIXME: maybe try to parse the date and catch exceptions
        }
    }
    catch (const boost::program_options::too_many_positional_options_error &)
    {
        std::stringstream msg;
        msg << "Error: Only one input file allowed." << std::endl << std::endl << desc;
        throw std::runtime_error(msg.str());
    }
    catch (const boost::program_options::error &e)
    {
        std::stringstream msg;
        msg << "Error: " << e.what() << std::endl << std::endl << desc;
        throw std::runtime_error(msg.str());
    }
    catch (const std::invalid_argument &e)
    {
        std::stringstream msg;
        msg << "Error: " << e.what();
        throw std::runtime_error(msg.str());
    }
    catch (...)
    {
        std::stringstream msg;
        msg << "Error: Unknown exception!" << std::endl << std::endl << desc;
        throw std::runtime_error(msg.str());
    }

    // argument --date is higher in priority, if set don't overwrite
    if (limit_to_date_str.empty())
    {
        // extract date from filename, so we have a clue which data we want
        // to extract from the file (it's possible that there is more than
        // one day data inside the file.
        boost::optional<std::string> igsdate = lcl_extractDateStringFromIGSFilename(filenameInput);
        if (igsdate)
        {
            limit_to_date_str = igsdate.get();
            std::cout << "Date limit from IGS filename: " << limit_to_date_str << std::endl;
        }
    }

    if (!limit_to_prn)
        throw std::runtime_error("Please limit to a specific SV!");

    if (limit_to_date_str.empty())
        throw std::runtime_error("Please limit to a specific day!");

    limit_to_date = bnav::DateTime(bnav::TimeSystem::BDT, limit_to_date_str + "T000000");
    std::cout << "Limiting date to: " << limit_to_date->getISODate() << std::endl;
}

void bnavMain::readInputFile()
{
    // Open file and parse lines
    bnav::AsciiReader reader(filenameInput, filetypeInput);
    if (!reader.isOpen())
        std::perror(("Error: Could not open file: " + filenameInput).c_str());

    uint32_t weeknum {0};
    uint32_t intervalCountOld = UINT32_MAX;
    bnav::Ionosphere iono_old;
    bnav::KlobucharParam klob_old;
    bnav::MessageStatistic msgstat;

    bnav::AsciiReaderEntry data;
    while (reader.readLine(data))
    {
#if 0
        // skip B2 signals, the differences are not in our interest
        if (data.getSignalType() != bnav::SignalType::BDS_B1)
            continue;
#endif

        const bnav::SvID sv(data.getPRN());

        if (limit_to_prn && sv != limit_to_prn.get())
            continue;

        bnav::Subframe sf(sv, data.getBits());

        // store only messages into stat, if we have a correct BeiDou date
        if (weeknum != 0)
        {
            const bnav::DateTime bdt = bnav::DateTime(bnav::TimeSystem::BDT, weeknum, sf.getSOW());
            msgstat.add(sv, bdt);
        }

#if 0
        // debug
        std::cout << "prn: " << std::setw(2) << data.getPRN()
                  << " wn: " << std::setw(4) << data.getDateTime().getWeekNum()
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
            uint32_t intervalCount = eph.getSOW() / limit_to_interval_klobuchar;
            // FIXME: we take only PRN 2 data here, if we would like to
            // replace missing data of prn 2 with other geos we have to
            // think about IonosphereStore, which stores in depending on SvID!
            if (limit_to_prn && sv == limit_to_prn.get() && intervalCount != intervalCountOld)
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
                    uint32_t secondOfInterval = eph.getSOW() % limit_to_interval_klobuchar;
                    uint32_t sowFullInterval = eph.getSOW() - secondOfInterval;
                    bnav::DateTime ephdate { bnav::TimeSystem::BDT, weeknum, sowFullInterval };
                    bnav::Ionosphere ionoklob(klob, ephdate, generateGlobalKlobuchar);

                    std::cout << klob << std::endl;
                    //ionoklob.dump();

                    if (limit_to_date && limit_to_date->isSameIonexDay(ionoklob.getDateOfIssue()))
                    {
                        std::cout << "add Klobuchar to store for SV: " << sv.getPRN() << " at " << ionoklob.getDateOfIssue().getDateTimeString() << std::endl;
                        ionostoreKlobuchar.addIonosphere(sv, ionoklob);
                    }

                    klob_old = klob;
                }
            }
        }
        else if (sfbuf->isAlmanacComplete())
        {
            const bnav::SubframeBufferParam bdata = sfbuf->flushAlmanacData();
            //std::cout << "almanac complete" << std::endl;

            // only Geos have Ionosphere
            if (sv.isGeo() && weeknum != 0)
            {
                bnav::Ionosphere iono(bdata, weeknum);

                // diff only for one single prn
                if (limit_to_prn && sv == limit_to_prn.get() && iono.getDateOfIssue().getSOW() % limit_to_interval_regional == 0)
                {
//                  if (iono_old.hasData())
//                      iono.diffToModel(iono_old).dump();

                    //iono.dump();

                    if (limit_to_date && limit_to_date->isSameIonexDay(iono.getDateOfIssue()))
                    {
                        std::cout << "add Regional Grid to store for SV: " << sv.getPRN() << " at " << iono.getDateOfIssue().getDateTimeString() << std::endl;
                        ionostore.addIonosphere(sv, iono);
                    }

                    iono_old = iono;
                }
            }
        }
    }
    reader.close();

    if (sbstore.hasIncompleteData())
        std::cout << "SubframeBufferStore has incomplete data sets at EOF. Ignoring." << std::endl;

    ionostore.dumpStoreStatistics("Regional grid");
    ionostoreKlobuchar.dumpStoreStatistics("Klobuchar");

    // dump message statistic
    msgstat.dump();

    // works only with one sv selected at the moment
    if (limit_to_prn)
    {
        if (ionostore.hasDataForSv(limit_to_prn.get()))
        {
            ionostore.dumpGridAvailability(limit_to_prn.get());

            if (!filenameIonexRegional.empty())
                writeIonexFile(filenameIonexRegional, limit_to_interval_regional, false);
        }
        else
        {
            std::cout << "No data in Regional Grid store. No Ionex output." << std::endl;
        }

        if (ionostoreKlobuchar.hasDataForSv(limit_to_prn.get()))
        {
            if (!filenameIonexKlobuchar.empty())
                writeIonexFile(filenameIonexKlobuchar, limit_to_interval_klobuchar, true);
        }
        else
        {
            std::cout << "No data in Klobuchar store. No Ionex output." << std::endl;
        }
    }
}

void bnavMain::writeIonexFile(const std::string &filename, const std::uint32_t interval, const bool klobuchar)
{
    std::cout << "Writing Ionex file: " << filename << std::endl;
    assert(limit_to_prn); // atm we can only store data from one prn
    // overwrites without warnings
    bnav::IonexWriter writer(filename, interval, klobuchar);
    if (!writer.isOpen())
        std::perror(("Error: Could not open file: " + filename).c_str());

    // write all models from prn
    const auto prn2data = klobuchar ? ionostoreKlobuchar.getItemsBySv(limit_to_prn.get()) : ionostore.getItemsBySv(limit_to_prn.get());
    if (prn2data)
        writer.writeAll(prn2data.get());
    else
        std::cerr << "writeIonexFile: no data for getItemsBySv!" << std::endl;
    writer.close();
}

} // namespace bnav
