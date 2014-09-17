#include "IonexWriter.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

//FIXME: Leica SpiderQC can only read the IONEX file, if there are data records
// at every! point defined by the Interval in the header!
// Solution: Write only 9999 for missing models.

namespace
{

const std::string author { "TUD/Arnhold" };
const std::string application { "bapp v0.4" };

// BeiDou specific
const std::vector<std::string> height {"475.0", "475.0", "0.0"};
const std::string radius { "6478.0" };

const std::vector<std::string> description_klobuchar = {
    "BeiDou Ionospheric Map (CIM), Klobuchar model",
    "Gridded GIM, based upon 8 parameter Klobuchar parameters.",
    "Vertical ionospheric delay, elevation set to 90 degrees",
    "For calculation details see:",
    "BDS ICD 2.0, 5.2.4.7 Ionospheric Delay Model Param., 2013"
};
const std::vector<std::string> description_regional = {
    "BeiDou Ionospheric Map, regional grid",
    "Read from D2 navigation message.",
    "BDS ICD 2.0, 5.3.3.8 Ionospheric Grid Information, 2013"
};

std::string lcl_justifyRight(const std::string &str, const std::string::size_type length, const char padding = ' ')
{
    // nothing to do
    if (str.length() == length)
        return str;

    // if string is longer, truncate
    if (str.length() > length)
        return str.substr(0, length);

    // fill left side with padding char
    return std::string(length - str.length(), padding) + str;
}

std::string lcl_justifyLeft(const std::string &str, const std::string::size_type length, const char padding = ' ')
{
    // nothing to do
    if (str.length() == length)
        return str;

    // if string is longer, truncate
    if (str.length() > length)
        return str.substr(0, length);

    // fill right side with padding char
    return str + std::string(length - str.length(), padding);
}

/// Wrapper for double values, converted to string as fixed values, e.g. 7.0
std::string lcl_justifyRight(const double num, const std::string::size_type length, const int32_t precision, const char padding = ' ')
{
    std::stringstream ss;
    ss << std::setprecision(precision) << std::fixed << num;

    return lcl_justifyRight(ss.str(), length, padding);
}

#if 0
std::string lcl_justifyLeft(const double num, const std::string::size_type length, const int32_t precision, const char padding = ' ')
{
    std::stringstream ss;
    ss << std::setprecision(precision) << std::fixed << num;

    return lcl_justifyLeft(ss.str(), length, padding);
}
#endif

static const bnav::Ionosphere lcl_dummyModel(const bnav::Ionosphere &data, const bnav::DateTime &dt)
{
    bnav::IonoGridDimension dim = data.getGridDimension();
    bnav::Ionosphere ionoref;
    ionoref.setGridDimension(dim);
    ionoref.setDateOfIssue(dt);
    // get grid and reuse tecu as counter ;)
    std::vector<bnav::IonoGridInfo> igpref = ionoref.getGrid();

    // zero initialize every single IGP
    for (auto it = igpref.begin(); it != igpref.end(); ++it)
        it->setVerticalDelay_TECU(9999);

    ionoref.setGrid(igpref);

    return ionoref;
}

}

namespace bnav
{

IonexWriter::IonexWriter()
    : m_outfile()
    , m_filename()
    , m_interval(UINT32_MAX)
    , m_isKlobuchar(false)
    , m_isHeaderWritten(false)
    , m_tecmapcount(0)
{
}

IonexWriter::IonexWriter(const char *filename, const std::size_t interval, const bool klobuchar)
    : m_outfile()
    , m_filename(filename)
    , m_interval(interval)
    , m_isKlobuchar(klobuchar)
    , m_isHeaderWritten(false)
    , m_tecmapcount(0)
{
    open(filename);
}


IonexWriter::IonexWriter(const std::string &filename, const std::size_t interval, const bool klobuchar)
    : IonexWriter(filename.c_str(), interval, klobuchar)
{
}

IonexWriter::~IonexWriter()
{
    // automatically close object on destruction
    if (isOpen())
        m_outfile.close();
}

void IonexWriter::open(const char *filename)
{
    // ensure there is no open file stream
    assert(!isOpen());

    m_filename = filename;
    m_outfile.open(filename, std::ofstream::out);
}

void IonexWriter::open(const std::string &filename)
{
    open(filename.c_str());
}

void IonexWriter::close()
{
    // ensure file stream is opened
    assert(isOpen());
    m_outfile.close();
}

bool IonexWriter::isOpen() const
{
    return m_outfile.is_open();
}

bool IonexWriter::isKlobuchar() const
{
    return m_isKlobuchar;
}

void IonexWriter::setKlobuchar(const bool klobuchar)
{
    m_isKlobuchar = klobuchar;
}

/// called from writeData, because we need at least one data record to get
/// all information of the map.
void IonexWriter::writeHeader(const Ionosphere &firstion, const Ionosphere &lastion)
{
    assert(isOpen());

    m_outfile << lcl_justifyRight("1.0", 8) << lcl_justifyLeft("", 12)
              << lcl_justifyLeft("IONOSPHERE MAPS", 20)
              << lcl_justifyLeft("BDS", 20)
              << "IONEX VERSION / TYPE" << std::endl;


    DateTime dtnow;
    dtnow.setToCurrentDateTimeUTC();
    m_outfile << lcl_justifyLeft(application, 20)
              << lcl_justifyLeft(author, 20)
              << lcl_justifyLeft(dtnow.getIonexDate(), 20)
              << "PGM / RUN BY / DATE" << std::endl;

    const std::vector<std::string> description { m_isKlobuchar ? description_klobuchar : description_regional };
    for (auto it = description.cbegin(); it != description.end(); ++it)
        m_outfile << lcl_justifyLeft(*it, 60)
                  << lcl_justifyLeft("DESCRIPTION", 20) << std::endl;

    DateTime dtfirst = firstion.getDateOfIssue();
    m_outfile << lcl_justifyRight(dtfirst.getYearString(), 6)
              << lcl_justifyRight(dtfirst.getMonthString(), 6)
              << lcl_justifyRight(dtfirst.getDayString(), 6)
              << lcl_justifyRight(dtfirst.getHourString(), 6)
              << lcl_justifyRight(dtfirst.getMinuteString(), 6)
              << lcl_justifyRight(dtfirst.getSecondString(), 6)
              << lcl_justifyRight("", 24)
              << lcl_justifyLeft("EPOCH OF FIRST MAP", 20)
              << std::endl;

    DateTime dtlast = lastion.getDateOfIssue();
    m_outfile << lcl_justifyRight(dtlast.getYearString(), 6)
              << lcl_justifyRight(dtlast.getMonthString(), 6)
              << lcl_justifyRight(dtlast.getDayString(), 6)
              << lcl_justifyRight(dtlast.getHourString(), 6)
              << lcl_justifyRight(dtlast.getMinuteString(), 6)
              << lcl_justifyRight(dtlast.getSecondString(), 6)
              << lcl_justifyRight("", 24)
              << lcl_justifyLeft("EPOCH OF LAST MAP", 20)
              << std::endl;

    m_outfile << lcl_justifyRight(std::to_string(m_interval), 6) << lcl_justifyLeft("", 54)
              << lcl_justifyLeft("INTERVAL", 20) << std::endl;

    // calculate total map count from interval, because we fill non existing
    // data records with dummies (9999).
    uint32_t totalsec = static_cast<uint32_t>(std::abs((lastion.getDateOfIssue() - firstion.getDateOfIssue()).total_seconds()));
    uint32_t mapcount = totalsec / m_interval + 1; // fence posts
    m_outfile << lcl_justifyRight(std::to_string(mapcount), 6) << lcl_justifyLeft("", 54)
              << lcl_justifyLeft("# OF MAPS IN FILE", 20) << std::endl;

    m_outfile << lcl_justifyLeft("", 2) << lcl_justifyLeft("NONE", 58)
              << lcl_justifyLeft("MAPPING FUNCTION", 20) << std::endl;

    // minimum elevation: 0.0 is unknown
    m_outfile << lcl_justifyRight("0.0", 8) << lcl_justifyLeft("", 52)
              << lcl_justifyLeft("ELEVATION CUTOFF", 20) << std::endl;

    // blank line: theoretical model
    m_outfile << lcl_justifyLeft("", 60)
              << lcl_justifyLeft("OBSERVABLES USED", 20) << std::endl;

    m_outfile << lcl_justifyRight(radius, 8) << lcl_justifyLeft("", 52)
              << lcl_justifyLeft("BASE RADIUS", 20) << std::endl;

    m_outfile << lcl_justifyRight("2", 6) << lcl_justifyLeft("", 54)
              << lcl_justifyLeft("MAP DIMENSION", 20) << std::endl;

    m_outfile << lcl_justifyLeft("", 2)
              << lcl_justifyRight(height[0], 6)
              << lcl_justifyRight(height[1], 6)
              << lcl_justifyRight(height[2], 6)
              << lcl_justifyLeft("", 40)
              << lcl_justifyLeft("HGT1 / HGT2 / DHGT", 20) << std::endl;

    const IonoGridDimension igd = firstion.getGridDimension();
    m_outfile << lcl_justifyLeft("", 2)
              << lcl_justifyRight(igd.latitude_north, 6, 1)
              << lcl_justifyRight(igd.latitude_south, 6, 1)
              << lcl_justifyRight(igd.latitude_spacing, 6, 1)
              << lcl_justifyLeft("", 40)
              << lcl_justifyLeft("LAT1 / LAT2 / DLAT", 20) << std::endl;

    m_outfile << lcl_justifyLeft("", 2)
              << lcl_justifyRight(igd.longitude_west, 6, 1)
              << lcl_justifyRight(igd.longitude_east, 6, 1)
              << lcl_justifyRight(igd.longitude_spacing, 6, 1)
              << lcl_justifyLeft("", 40)
              << lcl_justifyLeft("LON1 / LON2 / DLON", 20) << std::endl;

    m_outfile << lcl_justifyRight("-1", 6) << lcl_justifyLeft("", 54)
              << lcl_justifyLeft("EXPONENT", 20) << std::endl;

    m_outfile << lcl_justifyLeft("TEC values in 0.1 tec units; 9999, if no value available", 60)
              << lcl_justifyLeft("COMMENT", 20) << std::endl;

    m_outfile << lcl_justifyLeft("", 60)
              << lcl_justifyLeft("END OF HEADER", 20) << std::endl;

    m_isHeaderWritten = true;
}

/**
 * @brief IonexWriter::writeAll Master mind method, writes header, records and
 * finalizes the file.
 * @param data Ionospheric models.
 */
void IonexWriter::writeAll(const std::map<DateTime, Ionosphere> &data)
{
    assert(data.size() >= 1);
    Ionosphere firstion = data.begin()->second;
    Ionosphere lastion = data.rbegin()->second;

    // write header
    writeHeader(firstion, lastion);

    // run through all models
    std::uint32_t lastsec { 0 };
    std::uint32_t lastweek { 0 };
    // use a separate boolean, because SOW could be zero
    bool firstrun { true };
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        if (firstrun)
        {
           lastsec = it->second.getDateOfIssue().getSOW();
           firstrun = false;
        }
        else
        {
            // look if we have to fill with fake entries, because there is a data gap
            std::size_t secdiff = it->second.getDateOfIssue().getSOW() - lastsec;
            std::size_t fillcount = secdiff / m_interval;
            assert(secdiff % m_interval == 0);
            // fill one less, because we want to fill "fences" not "fence posts"
            for (std::size_t i = 1; i < fillcount; ++i)
            {
                std::cout << "IonexWriter: Inserting dummy entry" << std::endl;

                DateTime dt(TimeSystem::BDT, lastweek, lastsec + m_interval * i);
                writeRecord(std::make_pair(dt, lcl_dummyModel(it->second, dt)));
            }
        }

        // write true entry
        writeRecord(*it);
        lastsec = it->second.getDateOfIssue().getSOW();
        lastweek = it->second.getDateOfIssue().getWeekNum();
    }

    // finalize file
    finalize();
}

void IonexWriter::writeRecord(const std::pair<const DateTime, Ionosphere> &data)
{
    assert(isOpen());
    assert(m_isHeaderWritten);

    ++m_tecmapcount;

    IonoGridDimension igd = data.second.getGridDimension();
    std::vector<IonoGridInfo> grid = data.second.getGrid();
    DateTime dt = data.second.getDateOfIssue();

    const std::size_t colcount = igd.getItemCountLongitude();
    const std::size_t rowcount = igd.getItemCountLatitude();

    // set an arbitrary limit
    assert(colcount < 360);
    assert(rowcount < 360);

    // START OF TEC MAP: with index number
    m_outfile << lcl_justifyRight(std::to_string(m_tecmapcount), 6)
              << lcl_justifyRight("", 54)
              << lcl_justifyLeft("START OF TEC MAP", 20)
              << std::endl;

    // EPOCH OF CURRENT MAP
    m_outfile << lcl_justifyRight(dt.getYearString(), 6)
              << lcl_justifyRight(dt.getMonthString(), 6)
              << lcl_justifyRight(dt.getDayString(), 6)
              << lcl_justifyRight(dt.getHourString(), 6)
              << lcl_justifyRight(dt.getMinuteString(), 6)
              << lcl_justifyRight(dt.getSecondString(), 6)
              << lcl_justifyRight("", 24)
              << lcl_justifyLeft("EPOCH OF CURRENT MAP", 20)
              << std::endl;

    std::size_t index = 0;
    for (std::size_t row = 0; row < rowcount; ++row)
    {
        bool lastwasnewline = false;
        // header of one latitude record
        m_outfile << "  "
                  << lcl_justifyRight(igd.latitude_north + row * igd.latitude_spacing, 6, 1)
                  << lcl_justifyRight(igd.longitude_west, 6, 1)
                  << lcl_justifyRight(igd.longitude_east, 6, 1)
                  << lcl_justifyRight(igd.longitude_spacing, 6, 1)
                  << lcl_justifyRight(height[0], 6)
                  << lcl_justifyRight("", 28)
                  << lcl_justifyLeft("LAT/LON1/LON2/DLON/H", 20)
                  << std::endl;

        // data
        for (std::size_t col = 0; col < colcount; ++col)
        {
            m_outfile << std::setw(5) << grid[index].getVerticalDelay_TECU();

            // we have to insert a break after every 16 enries
            if (col > 0 && (col + 1) % 16 == 0)
            {
                lastwasnewline = true;
                m_outfile << std::endl;
            }
            else
                lastwasnewline = false;

            ++index;
        }

        // don't insert two newlines, if we did insert a break after the 16th
        // entrie (in case we have exactly 16, 32,... entries for a record.
        if (!lastwasnewline)
            m_outfile << std::endl;
    }

    // END OF TEC MAP: with index number
    m_outfile << lcl_justifyRight(std::to_string(m_tecmapcount), 6)
              << lcl_justifyRight("", 54)
              << lcl_justifyLeft("END OF TEC MAP", 20)
              << std::endl;
}

void IonexWriter::finalize()
{
    assert(m_tecmapcount != 0);
    m_outfile << lcl_justifyRight("", 60)
              << lcl_justifyLeft("END OF FILE", 20)
              << std::endl;
}

} // namespace bnav
