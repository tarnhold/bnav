#ifndef IONOSPHERESTORE_H
#define IONOSPHERESTORE_H

#include "Ionosphere.h"
#include "SvID.h"

#include <map>
#include <stdint.h>

namespace bnav
{

//TODO: class template -> eph,alm,..Store are all nearly identical
class IonosphereStore
{
    std::map<SvID, std::map< uint32_t, Ionosphere > > m_store;

public:
    IonosphereStore();
    ~IonosphereStore();

    void addIonosphere(const SvID &sv, const Ionosphere &iono);

    //Ionosphere* getIonosphere(const SvID);

private:
    void addSvID(const SvID &sv);
};

} // namespace bnav

#endif // IONOSPHERESTORE_H
