#ifndef IONOSPHERESTORE_H
#define IONOSPHERESTORE_H

#include "Ionosphere.h"
#include "DateTime.h"
#include "SvID.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace bnav
{

//TODO: class template -> eph,alm,..Store are all nearly identical
class IonosphereStore
{
    std::map<SvID, std::map< DateTime, Ionosphere > > m_store;

public:
    IonosphereStore();

    void addIonosphere(const SvID &sv, const Ionosphere &iono);

    [[nodiscard]] bool hasDataForSv(const SvID &sv) const;

    [[nodiscard]] std::vector< SvID > getSvList() const;
    [[nodiscard]] std::optional< std::map<DateTime, Ionosphere> > getItemsBySv(const SvID &sv) const;
    [[nodiscard]] std::optional< Ionosphere > getIonosphere(const SvID &sv, const DateTime &datetime) const;

    void dumpStoreStatistics(const std::string &name) const;
    void dumpGridAvailability(const SvID &sv) const;
};

} // namespace bnav

#endif // IONOSPHERESTORE_H
