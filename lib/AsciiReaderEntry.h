#ifndef ASCIIREADERENTRY_H
#define ASCIIREADERENTRY_H

#include "BeiDou.h"
#include "DateTime.h"
#include "NavBits.h"

#include <string>
#include <string_view>

namespace bnav
{

/// Base type for line element
class AsciiReaderEntry
{
protected:
    uint32_t m_prn;
    DateTime m_datetime;
    SignalType m_sigtype;
    NavBits<300> m_bits;

public:
    AsciiReaderEntry();
    AsciiReaderEntry(std::string_view);

    void readLine(std::string_view line);

    uint32_t getPRN() const;
    DateTime getDateTime() const;
    SignalType getSignalType() const;
    NavBits<300> getBits() const;
};

// Type for JPS style files
class AsciiReaderEntryJPS final : public AsciiReaderEntry
{
public:
    AsciiReaderEntryJPS();
    AsciiReaderEntryJPS(std::string_view line);

    void readLine(std::string_view line);
};

// Type for SBF style files
class AsciiReaderEntrySBF final : public AsciiReaderEntry
{
public:
    AsciiReaderEntrySBF();
    AsciiReaderEntrySBF(std::string_view line);

    void readLine(std::string_view line);
};

// Type for SBF style files
class AsciiReaderEntrySBFHex final : public AsciiReaderEntry
{
public:
    AsciiReaderEntrySBFHex();
    AsciiReaderEntrySBFHex(std::string_view line);

    void readLine(std::string_view line);
};

} // namespace bnav

#endif // ASCIIREADERENTRY_H
