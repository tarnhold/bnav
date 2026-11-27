#ifndef BNAVMAIN_H
#define BNAVMAIN_H

#include "AsciiReader.h"
#include "DateTime.h"
#include "IonosphereStore.h"
#include "SubframeBufferStore.h"
#include "SvID.h"

#include <cstdint>
#include <optional>
#include <string>

namespace bnav
{

class bnavMain final
{
    bnavMain(const bnavMain &) = delete;
    bnavMain &operator=(const bnavMain &) = delete;

    std::string filenameInput;
    bnav::AsciiReaderType filetypeInput;
    std::string filenameIonexKlobuchar;
    std::string filenameIonexRegional;

    bool generateGlobalKlobuchar;
    bool m_showHelp;
    std::uint32_t limit_to_interval_regional;
    std::uint32_t limit_to_interval_klobuchar;
    std::optional<SvID> limit_to_prn;
    std::optional<DateTime> limit_to_date;

    bnav::SubframeBufferStore sbstore;
    bnav::IonosphereStore ionostore;
    bnav::IonosphereStore ionostoreKlobuchar;

public:
    bnavMain(int argc, char *argv[]);

    [[nodiscard]] bool helpRequested() const;
    void readInputFile();

private:
    void writeIonexFile(const std::string &filename, const std::uint32_t interval, const bool klobuchar);
};

} // namespace bnav

#endif // BNAVMAIN_H
