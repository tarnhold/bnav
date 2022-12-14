#ifndef BNAVMAIN_H
#define BNAVMAIN_H

#include "AsciiReader.h"
#include "IonosphereStore.h"
#include "SubframeBufferStore.h"
#include "SvID.h"

#include <string>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

namespace bnav
{

class bnavMain final: public boost::noncopyable
{
    std::string filenameInput;
    bnav::AsciiReaderType filetypeInput;
    std::string filenameIonexKlobuchar;
    std::string filenameIonexRegional;

    bool generateGlobalKlobuchar;
    std::uint32_t limit_to_interval_regional;
    std::uint32_t limit_to_interval_klobuchar;
    boost::optional<SvID> limit_to_prn;
    boost::optional<DateTime> limit_to_date;

    bnav::SubframeBufferStore sbstore;
    bnav::IonosphereStore ionostore;
    bnav::IonosphereStore ionostoreKlobuchar;

public:
    bnavMain(int argc, char *argv[]);

    void readInputFile();

private:
    void writeIonexFile(const std::string &filename, const std::uint32_t interval, const bool klobuchar);
};

} // namespace bnav

#endif // BNAVMAIN_H
