#ifndef ASCIIREADERNAVENTRY_H
#define ASCIIREADERNAVENTRY_H

#include "AsciiReader.h"
#include "BDSCommon.h"
#include "NavBits.h"

namespace bnav
{

/// Base type for line element
class ReaderNavEntry
{
protected:
    uint32_t m_prn;
    uint32_t m_tow;
    SignalType m_sigtype;
    NavBits<300> m_bits;

public:
    ReaderNavEntry();
    ReaderNavEntry(const std::string &);

    void readLine(const std::string &line);

    uint32_t getTOW() const;
    uint32_t getPRN() const;
    SignalType getSignalType() const;
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

} // namespace bnav

#endif // ASCIIREADERNAVENTRY_H
