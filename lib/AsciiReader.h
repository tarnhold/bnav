#ifndef ASCIIREADER_H
#define ASCIIREADER_H

#include "AsciiReaderEntry.h"

#include <string>
#include <fstream>

#include <boost/noncopyable.hpp>

namespace bnav
{

enum class AsciiReaderType
{
    TEXT_CONVERTED_JPS,
    TEXT_CONVERTED_SBF,
    TEXT_CONVERTED_SBF_HEX,
    NONE
};

/**
Class for handling the input file stream by line.
*/
class AsciiReader : private boost::noncopyable
{
private:
    std::ifstream m_infile; ///< Input file stream
    std::string m_filename; ///< File name
    AsciiReaderType m_filetype; ///< Type of source file (sbf, jps)
    bool m_eof; ///< State if EOF is reached

public:
    AsciiReader();
    AsciiReader(const char *filename, const AsciiReaderType &filetype);
    AsciiReader(const std::string &filename, const AsciiReaderType &filetype);
    ~AsciiReader();

    void open(const char *filename);
    void open(const std::string &filename);
    bool isOpen() const;

    void setType(const AsciiReaderType &filetype);
    AsciiReaderType getType() const;

    /// Read current line, return data by reference
    bool readLine(AsciiReaderEntry &data);
    bool isEof() const;
    void close();
};

} // namespace bnav

#endif // ASCIIREADER_H
