#include "AsciiReader.h"
#include "AsciiReaderEntry.h"
#include "Debug.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <string>

#include <boost/algorithm/string.hpp>

namespace bnav
{

AsciiReader::AsciiReader()
    : m_infile()
    , m_filename()
    , m_filetype(AsciiReaderType::NONE)
    , m_eof(false)
{
}

AsciiReader::AsciiReader(const char *filename, const AsciiReaderType &filetype)
    : m_infile()
    , m_filename(filename)
    , m_filetype(filetype)
    , m_eof(false)
{
    open(filename);
}

AsciiReader::AsciiReader(const std::string &filename, const AsciiReaderType &filetype)
    : AsciiReader(filename.c_str(), filetype)
{
}

AsciiReader::~AsciiReader()
{
    // automatically close object on destruction
    if (isOpen())
        m_infile.close();
}

bool AsciiReader::isOpen() const
{
    return m_infile.is_open();
}

void AsciiReader::open(const char *filename)
{
    // ensure there is no open file stream
    assert(!isOpen());

    // ensure filetype is set
    assert(m_filetype != AsciiReaderType::NONE);

    m_filename = filename;
    m_infile.open(filename, std::ifstream::in);
}

void AsciiReader::open(const std::string &filename)
{
    open(filename.c_str());
}

void AsciiReader::setType(const AsciiReaderType &filetype)
{
    m_filetype = filetype;
}

AsciiReaderType AsciiReader::getType() const
{
    return m_filetype;
}

/**
 * @brief AsciiReader::readLine Read one line
 * @param data ReaderEntry data type
 * @return  true if line read was succesful.
 */
bool AsciiReader::readLine(AsciiReaderEntry &data)
{
    if (isEof())
        return false;

    std::string line;
    std::getline(m_infile, line);

    if (m_infile.bad())
    {
        std::perror(("Error while reading file: " + m_filename).c_str());
        return false;
    }

    // assume empty line is also eof
    if (line.empty())
        return false;

    if (m_filetype == AsciiReaderType::TEXT_CONVERTED_JPS)
        data = AsciiReaderEntryJPS(line);
    else if (m_filetype == AsciiReaderType::TEXT_CONVERTED_SBF)
        data = AsciiReaderEntrySBF(line);
    else if (m_filetype == AsciiReaderType::TEXT_CONVERTED_SBF_HEX)
        data = AsciiReaderEntrySBFHex(line);

    if (m_infile.eof())
        m_eof = true;

    return true;
}

bool AsciiReader::isEof() const
{
    return m_eof;
}

void AsciiReader::close()
{
    // ensure file stream is opened
    assert(isOpen());
    m_infile.close();
}

} // namespace bnav
