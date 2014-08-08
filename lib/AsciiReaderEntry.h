#ifndef ASCIIREADERENTRY_H
#define ASCIIREADERENTRY_H

#include "AsciiReader.h"
#include "BeiDou.h"
#include "NavBits.h"

namespace bnav
{

/// Base type for line element
class AsciiReaderEntry
{
protected:
    uint32_t m_prn;
    uint32_t m_tow;
    SignalType m_sigtype;
    NavBits<300> m_bits;

public:
    AsciiReaderEntry();
    AsciiReaderEntry(const std::string &);

    void readLine(const std::string &line);

    uint32_t getTOW() const;
    uint32_t getPRN() const;
    SignalType getSignalType() const;
    NavBits<300> getBits() const;
};

// Type for JPS style files
class AsciiReaderEntryJPS : public AsciiReaderEntry
{
public:
    AsciiReaderEntryJPS();
    AsciiReaderEntryJPS(const std::string &line);

    void readLine(const std::string &line);
};

// Type for SBF style files
class AsciiReaderEntrySBF : public AsciiReaderEntry
{
public:
    AsciiReaderEntrySBF();
    AsciiReaderEntrySBF(const std::string &line);

    void readLine(const std::string &line);
};

} // namespace bnav

#endif // ASCIIREADERENTRY_H
