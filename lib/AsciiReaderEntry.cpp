#include "AsciiReader.h"
#include "AsciiReaderEntry.h"
#include "BDSCommon.h"

#include "Debug.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string.hpp>

namespace
{

/*
 * References
 *
 * [1] Septentrio SBF Reference Guide v1.15.3
 * [2] Septentrio RxTools User Manual v1.10.0
 *
 */

/*
 * extract data by keyword from line
 */
static std::string extractData(const std::string &line, const std::string &keyword)
{
    // example: tow 310190 PRN 5 len 10 [data 09e345e3 ...]
    const std::size_t pos = line.find(keyword);

    // keyword not found
    if (pos == std::string::npos)
        throw std::invalid_argument("keyword \"" + keyword + "\" not found");

    // start of 310190
    const std::size_t pos_start = pos + keyword.length();
    // find next whitespace after 310190
    const std::size_t pos_end = line.find(' ', pos_start);

    if (pos_start == std::string::npos || pos_end == std::string::npos)
        throw std::invalid_argument("invalid data line");

    return line.substr(pos_start, pos_end - pos_start);
}

} // namespace anonymous

namespace bnav
{

ReaderNavEntry::ReaderNavEntry()
    : m_prn(0)
    , m_tow(0)
    , m_sigtype(SignalType::NONE)
    , m_bits()
{
}

ReaderNavEntry::ReaderNavEntry(const std::string &/*line*/)
    : m_prn(0)
    , m_tow(0)
    , m_sigtype(SignalType::NONE)
    , m_bits()
{
}

uint32_t ReaderNavEntry::getTOW() const
{
    return m_tow;
}

uint32_t ReaderNavEntry::getPRN() const
{
    return m_prn;
}

SignalType ReaderNavEntry::getSignalType() const
{
    return m_sigtype;
}

NavBits<300> ReaderNavEntry::getBits() const
{
    return m_bits;
}

void ReaderNavEntry::readLine(const std::string &)
{
    // should not be called
    assert(false);
}


/*
 * Reads an ASCII file with format
 * tow [tow] PRN [prn] len [len] data [data]
 *
 * Returns a bitset of the raw navigation message
 */
ReaderNavEntryJPS::ReaderNavEntryJPS()
{
}

ReaderNavEntryJPS::ReaderNavEntryJPS(const std::string &line)
{
    readLine(line);
}

void ReaderNavEntryJPS::readLine(const std::string &line)
{
    // parse tow and prn fields
    try
    {
        m_tow = std::stoul(extractData(line, "tow "));
        m_prn = std::stoul(extractData(line, "PRN "));
    }
    catch (std::invalid_argument)
    {
        std::cerr << "Malformed data line. Wrong format?" << std::endl;
        exit(1);
    }

    //DEBUG("tow:" << m_tow);
    //DEBUG("prn:" << m_prn);

    // assume JPS is only B1 signal
    m_sigtype = SignalType::BDS_B1;

    // get data field
    const std::string strdata("data ");
    const std::size_t pos = line.find(strdata);
    if (pos == std::string::npos)
    {
        std::cerr << "Malformed data line. Wrong format?" << std::endl;
        exit(1);
    }

    std::string hexdata = line.substr(pos + strdata.length());

    // remove whitespaces from hex data
    hexdata.erase( std::remove_if(hexdata.begin(), hexdata.end(), isspace), hexdata.end() );


    // We have 80 hex characters, that is 40 hex values (two hex chars form
    // one 8 bit block -> 40*8=320).
    assert(hexdata.length() == 80);

    // loop through hex string by getting two characters at a time
    NavBits<320> navbits320;
    for (std::size_t i = 0; i <= hexdata.length() - 2; i=i+2)
    {
        const uint32_t hexval = std::stoul(hexdata.substr(i, 2), nullptr, 16);
        const std::size_t bsize = 8;
        const NavBits<bsize> bitblock(hexval);

        // shift 8 bytes to the left and fill the right side
        navbits320 <<= bsize;
        for (std::size_t k = 0; k < bsize; ++k)
            navbits320[k] = bitblock[k];

        //std::cout << i << ": " << bitblock << std::endl;
    }

    // The last 20 bits have to be zero, because we have only 300 bits nav msg.
    NavBits<20> lastblock(navbits320.getLeft<300, 20>());
    assert(lastblock.to_ulong() == 0);

    m_bits = navbits320.getLeft<0, 300>();

    //std::cout << m_bits << std::endl;
}

/*
 * Reads an ASCII file with format
 * TOW,WNc,SvID,CRCPassed,..,Data
 *
 * Returns a bitset of the raw navigation message
 */
ReaderNavEntrySBF::ReaderNavEntrySBF()
{
}

ReaderNavEntrySBF::ReaderNavEntrySBF(const std::string &line)
{
    readLine(line);
}

void ReaderNavEntrySBF::readLine(const std::string &line)
{
    static const uint32_t SBF_SVID_OFFSET_BEIDOU = 140;

    // one line looks like:
    // 345605000,1801,145,1,28,3795932449 2099070704 0 0 0 0 0 0 0 1
    // which is
    // TOW [0.001 s], WNc [w], SVID, CRCPassed, signalType, NAVBits
    //
    // Reference: [1] 3.2 Navigation Page blocks

    // signalType
    // 28=CMP_B1
    // 29=CMP_B2
    //
    // Reference: [2] 11.9 sbf2ismr

    // split fields, delimited by comma
    std::vector<std::string> splitline;
    boost::split(splitline, line, boost::is_any_of(","));

    // ensure we have all elements
    assert(splitline.size() == 6);

    // parse tow and prn fields

    /*
     * SBF TOW represents every single subframe time stamp. This makes no
     * difference for D1, but for D2.
     *
     * Duration of one subframe:
     * D1: 6s
     * D2: 0.6s
     *
     * BDS SOW behavior:
     * D1: Every subframe gets a unique timestamp.
     * D2: Every _frame_ gets a unique timestamp. That means all subframes of
     *     that frame have the same SOW.
     *
     * Additionaly there is an offset between TOW and BDS SOW:
     * D1: 20s
     * D2: 14.4s + frameID * 0.6s
     */
    m_tow = std::stoul(splitline[0]);
    // according to SBF Ref Guide BeiDou Sv IDs have an offset of 140
    m_prn = std::stoul(splitline[2]) - SBF_SVID_OFFSET_BEIDOU;

    //DEBUG("tow: " << m_tow);
    //DEBUG("prn: " << m_prn);

    // determine signal type - yes, Septentrio saves both B1 and B2
    const uint32_t sigtype = std::stoul(splitline[4]);

    if (sigtype == 28)
        m_sigtype = SignalType::BDS_B1;
    else if (sigtype == 29)
        m_sigtype = SignalType::BDS_B2;

    // parse last field, which contains 10 numeric values, separated by whitespace
    std::vector<std::string> splitbits;
    boost::split(splitbits, splitline[5], boost::is_any_of(" "));

    // ensure we have 10 values
    assert(splitbits.size() == 10);

    NavBits<320> navbits320;
    for (std::vector<std::string>::iterator it = splitbits.begin(); it < splitbits.end(); ++it)
    {
        const uint32_t val = std::stoul(*it);
        const std::size_t bsize = 32;
        const NavBits<bsize> bitblock(val);

        // shift 32 bytes to the left and fill the right side
        navbits320 <<= bsize;
        for (std::size_t k = 0; k < bsize; ++k)
            navbits320[k] = bitblock[k];

        //std::cout << ": " << bitblock << std::endl;
    }

    // The last 20 bits have to be zero, because we have only 300 bits nav msg.
    // For whatever reason the last bit inside the SBF data is set to one
    // ignore this bit, by removing it with -1.
    NavBits<32> lastblock(std::stoul(splitbits[9]) - 1);
    lastblock <<= 12; // ignore the 12 msb which contain information
    assert(lastblock.to_ulong() == 0);


    m_bits = navbits320.getLeft<0, 300>();

    //std::cout << m_bits << std::endl;
}

} // namespace bnav