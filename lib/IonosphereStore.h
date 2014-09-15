#ifndef IONOSPHERESTORE_H
#define IONOSPHERESTORE_H

#include "Ionosphere.h"
#include "DateTime.h"
#include "SvID.h"

#include <map>
#include <stdint.h>

namespace bnav
{

//TODO: class template -> eph,alm,..Store are all nearly identical
class IonosphereStore
{
    std::map<SvID, std::map< DateTime, Ionosphere > > m_store;

public:
    IonosphereStore();
    ~IonosphereStore();

    void addIonosphere(const SvID &sv, const Ionosphere &iono);

    std::map< DateTime, Ionosphere> getItemsBySv(const SvID &sv);
    Ionosphere getIonosphere(const SvID &sv, const DateTime &datetime);

    void dumpGridAvailability(const SvID &sv);
};

} // namespace bnav

#endif // IONOSPHERESTORE_H
