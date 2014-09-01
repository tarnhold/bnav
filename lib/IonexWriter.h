#ifndef IONEXWRITER_H
#define IONEXWRITER_H

#include <fstream>
#include <string>

#include <boost/noncopyable.hpp>

namespace bnav
{

class IonexWriter : private boost::noncopyable
{
    std::ofstream m_outfile; ///< Output file stream
    std::string m_filename; ///< File name
    bool m_isGIM; ///< Write GIM or Regional model

public:
    IonexWriter();
    IonexWriter(const char *filename, const bool gim = true);
    IonexWriter(const std::string &filename, const bool gim = true);
    ~IonexWriter();

    void open(const char *filename);
    void open(const std::string &filename);

    bool isOpen() const;

    bool isGIM() const;
    void setGIM(const bool gim = true);

    void writeHeader();
    void writeData(/* parameter foo */);

    void close();

private:
    void finalizeData();
};

} // namespace bnav

#endif // IONEXWRITER_H
