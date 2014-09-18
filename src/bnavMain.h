#ifndef BNAVMAIN_H
#define BNAVMAIN_H

#include "AsciiReader.h"
#include "IonosphereStore.h"
#include "SubframeBufferStore.h"

#include <string>

#include <boost/optional.hpp>

namespace bnav
{

class bnavMain
{
    std::string filenameInput;
    bnav::AsciiReaderType filetypeInput;
    std::string filenameIonexKlobuchar;
    std::string filenameIonexRegional;

    bool generateGlobalKlobuchar;
    std::size_t limit_to_interval_regional;
    std::size_t limit_to_interval_klobuchar;
    std::size_t limit_to_prn;
    boost::optional<DateTime> limit_to_date;

    bnav::SubframeBufferStore sbstore;
    bnav::IonosphereStore ionostore;
    bnav::IonosphereStore ionostoreKlobuchar;

public:
    bnavMain(int argc, char *argv[]);

    void readInputFile();

private:
    void writeIonexFile(const std::string &filename, const std::size_t interval, const bool klobuchar);
};

} // namespace bnav

#endif // BNAVMAIN_H
