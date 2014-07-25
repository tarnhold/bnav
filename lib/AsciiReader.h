#ifndef ASCIIREADER_H
#define ASCIIREADER_H

#include "NavBits.h"

#include <string>
#include <fstream>
#include <vector>

#include <boost/noncopyable.hpp>

namespace bnav
{

enum AsciiReaderType
{
    TEXT_CONVERTED_JPS,
    TEXT_CONVERTED_SBF,
    NONE
};

/// Base type for line element
class ReaderNavEntry
{
protected:
    int m_prn;
    int m_tow;
    NavBits<300> m_bits;

public:
    ReaderNavEntry();
    ReaderNavEntry(const std::string &);

    void readLine(const std::string &line);

    int getTOW() const;
    int getPRN() const;
    NavBits<300> getBits() const;
};

// Type for JPS style files
class ReaderNavEntryJPS : public ReaderNavEntry
{
public:
    ReaderNavEntryJPS();
    ReaderNavEntryJPS(const std::string &line);

    void readLine(const std::string &line);
};

// Type for SBF style files
class ReaderNavEntrySBF : public ReaderNavEntry
{
public:
    ReaderNavEntrySBF();
    ReaderNavEntrySBF(const std::string &line);

    void readLine(const std::string &line);
};

/**
Class for handling the input file stream by line.
*/
class AsciiReader : private boost::noncopyable
{
private:
    std::ifstream m_infile; //< Input file stream
    AsciiReaderType m_filetype; //< Type of source file (sbf, jps)
    bool m_eof; //< State if EOF is reached

public:
    AsciiReader();
    AsciiReader(const char *filename, const AsciiReaderType &filetype);
    AsciiReader(const std::string &filename, const AsciiReaderType &filetype);
    ~AsciiReader();

    void open(const char *filename);
    void open(const std::string &filename);
    //void open(const std::string &filename, const AsciiReaderType &filetype);
    bool isOpen();

    void setType(const AsciiReaderType &filetype);
    AsciiReaderType getType();

    /// Read current line, return data by reference
    bool readLine(ReaderNavEntry &data);
    bool isEof();
    void close();
};

} // namespace bnav

#endif // ASCIIREADER_H
