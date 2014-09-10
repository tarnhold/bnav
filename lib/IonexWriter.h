#ifndef IONEXWRITER_H
#define IONEXWRITER_H

#include "DateTime.h"
#include "Ionosphere.h"

#include <fstream>
#include <string>
#include <map>

#include <boost/noncopyable.hpp>

namespace bnav
{

class IonexWriter : private boost::noncopyable
{
    std::ofstream m_outfile; ///< Output file stream
    std::string m_filename; ///< File name
    bool m_isGIM; ///< Write GIM or Regional model
    bool m_isHeaderWritten;
    std::size_t m_tecmapcount; ///< Index of current TEC map

public:
    IonexWriter();
    IonexWriter(const char *filename, const bool gim = false);
    IonexWriter(const std::string &filename, const bool gim = false);
    ~IonexWriter();

    void open(const char *filename);
    void open(const std::string &filename);

    bool isOpen() const;

    bool isGIM() const;
    void setGIM(const bool gim = true);

    void writeAll(const std::map<DateTime, Ionosphere> &data);

    void close();

private:
    void writeHeader(const Ionosphere &firstion, const Ionosphere &lastion, const std::size_t mapcount, const int32_t interval);
    void writeRecord(const std::pair<const DateTime, Ionosphere> &data);
    void finalize();
};

} // namespace bnav

#endif // IONEXWRITER_H
