#include "IonexWriter.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <vector>

namespace
{

const std::string author { "TUD/Arnhold" };
const std::string application { "bapp v0.4" };

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

}

namespace bnav
{

IonexWriter::IonexWriter()
    : m_outfile()
    , m_filename()
    , m_isGIM(true)
{
}

IonexWriter::IonexWriter(const char *filename, const bool isGIM)
    : m_outfile()
    , m_filename(filename)
    , m_isGIM(isGIM)
{
    open(filename);
}


IonexWriter::IonexWriter(const std::string &filename, const bool isGIM)
    : IonexWriter(filename.c_str(), isGIM)
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

    finalizeData();

    m_outfile.close();
}

bool IonexWriter::isOpen() const
{
    return m_outfile.is_open();
}

bool IonexWriter::isGIM() const
{
    return m_isGIM;
}

void IonexWriter::setGIM(const bool isGIM)
{
    m_isGIM = isGIM;
}

void IonexWriter::writeHeader()
{
    std::vector<std::string> latitude, longitude, description;

    if (m_isGIM)
    {
        // dimension like IGS
        latitude = {"87.5", "-87.5", "-2.5"};
        longitude = {"-180.0", "180.0", "5.0"};
        description = {
            "BeiDou Ionospheric Map (CIM), Klobuchar model",
            "Gridded GIM, based upon 8 parameter Klobuchar parameters.",
            "Vertical ionospheric delay, elevation set to 90 degrees",
            "For calculation details see:",
            "BDS ICD 2.0, 5.2.4.7 Ionospheric Delay Model Param., 2013",
        };
    }
    else
    {
        // regional model, CIM
        latitude = {"55.0", "7.5", "-2.5"};
        longitude = {"70.0", "145.0", "5.0"};
        description = {
            "BeiDou Ionospheric Map, regional grid",
            "Read from D2 navigation message.",
            "BDS ICD 2.0, 5.3.3.8 Ionospheric Grid Information, 2013",
        };
    }

    // BeiDou specific
    std::vector<std::string> height {"475.0", "475.0", "0.0"};
    std::string radius { "6478.0" };

    std::cout << lcl_justifyRight("1.0", 8) << lcl_justifyLeft("", 12)
              << lcl_justifyLeft("IONOSPHERE MAPS", 20)
              << lcl_justifyLeft("BDS", 20)
              << "IONEX VERSION / TYPE" << std::endl;


    std::cout << lcl_justifyLeft(application, 20)
              << lcl_justifyLeft(author, 20)
              << lcl_justifyLeft("PSEUDODATE", 20)
              << "PGM / RUN BY / DATE" << std::endl;

    for (auto it = description.cbegin(); it != description.end(); ++it)
        std::cout << lcl_justifyLeft(*it, 60)
                  << lcl_justifyLeft("DESCRIPTION", 20) << std::endl;

    // EPOCH OF FIRST MAP
    // EPOCH OF LAST MAP

    std::cout << lcl_justifyRight("7200", 6) << lcl_justifyLeft("", 54)
              << lcl_justifyLeft("INTERVAL", 20) << std::endl;

    // # OF MAPS IN FILE

    std::cout << lcl_justifyLeft("", 2) << lcl_justifyLeft("NONE", 58)
              << lcl_justifyLeft("MAPPING FUNCTION", 20) << std::endl;

    // minimum elevation: 0.0 is unknown
    std::cout << lcl_justifyRight("0.0", 8) << lcl_justifyLeft("", 52)
              << lcl_justifyLeft("ELEVATION CUTOFF", 20) << std::endl;

    // blank line: theoretical model
    std::cout << lcl_justifyLeft("", 60)
              << lcl_justifyLeft("OBSERVABLES USED", 20) << std::endl;

    std::cout << lcl_justifyRight(radius, 8) << lcl_justifyLeft("", 52)
              << lcl_justifyLeft("BASE RADIUS", 20) << std::endl;

    std::cout << lcl_justifyRight("2", 6) << lcl_justifyLeft("", 54)
              << lcl_justifyLeft("MAP DIMENSION", 20) << std::endl;

    std::cout << lcl_justifyLeft("", 2)
              << lcl_justifyRight(height[0], 6)
              << lcl_justifyRight(height[1], 6)
              << lcl_justifyRight(height[2], 6)
              << lcl_justifyLeft("", 40)
              << lcl_justifyLeft("HGT1 / HGT2 / DHGT", 20) << std::endl;

    std::cout << lcl_justifyLeft("", 2)
              << lcl_justifyRight(latitude[0], 6)
              << lcl_justifyRight(latitude[1], 6)
              << lcl_justifyRight(latitude[2], 6)
              << lcl_justifyLeft("", 40)
              << lcl_justifyLeft("LAT1 / LAT2 / DLAT", 20) << std::endl;

    std::cout << lcl_justifyLeft("", 2)
              << lcl_justifyRight(longitude[0], 6)
              << lcl_justifyRight(longitude[1], 6)
              << lcl_justifyRight(longitude[2], 6)
              << lcl_justifyLeft("", 40)
              << lcl_justifyLeft("LON1 / LON2 / DLON", 20) << std::endl;

    std::cout << lcl_justifyRight("-1", 6) << lcl_justifyLeft("", 54)
              << lcl_justifyLeft("EXPONENT", 20) << std::endl;

    std::cout << lcl_justifyLeft("TEC values in 0.1 tec units; 9999, if no value available", 60)
              << lcl_justifyLeft("COMMENT", 20) << std::endl;

    std::cout << lcl_justifyLeft("", 60)
              << lcl_justifyLeft("END OF HEADER", 20) << std::endl;
}

void IonexWriter::finalizeData()
{
    // write END OF TEC MAP stuff
}

} // namespace bnav
