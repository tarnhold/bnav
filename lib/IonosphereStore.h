#ifndef IONOSPHERESTORE_H
#define IONOSPHERESTORE_H

#include "Ionosphere.h"
#include "DateTime.h"
#include "SvID.h"

#include <map>
#include <stdint.h>

#include <boost/optional.hpp>

namespace bnav
{

//TODO: class template -> eph,alm,..Store are all nearly identical
class IonosphereStore
{
    std::map<SvID, std::map< DateTime, Ionosphere > > m_store;

public:
    IonosphereStore();

    void addIonosphere(const SvID &sv, const Ionosphere &iono);

    bool hasDataForSv(const SvID &sv) const;

    std::vector< SvID > getSvList() const;
    boost::optional< std::map<DateTime, Ionosphere> > getItemsBySv(const SvID &sv) const;
    boost::optional< Ionosphere > getIonosphere(const SvID &sv, const DateTime &datetime) const;

    void dumpStoreStatistics(const std::string name);
    void dumpGridAvailability(const SvID &sv);
};

} // namespace bnav

#endif // IONOSPHERESTORE_H
