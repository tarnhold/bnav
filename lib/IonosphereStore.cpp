#include "IonosphereStore.h"

namespace bnav
{

/**
 * @brief IonosphereStore::IonosphereStore Storage for Ionosphere objects.
 *
 * Used to manage multiple Ionosphere objects for multiple SVs.
 */
IonosphereStore::IonosphereStore()
    : m_store()
{
}

/**
 * @brief IonosphereStore::~IonosphereStore
 *
 * Clean up underlying Ionosphere objects if store gets destroyed.
 */
IonosphereStore::~IonosphereStore()
{
#if 0
    for (auto it = m_store.begin(); it != m_store.end(); ++it)
        delete it->second;
#endif
}

/**
 * @brief IonosphereStore::addIonosphere Add a Ionosphere to the storage.
 *
 * Automatically adds a new storage object, if a given SvID is not known.
 *
 * @param sv The SvID.
 * @param iono The Ionosphere object.
 */
void IonosphereStore::addIonosphere(const SvID &sv, const Ionosphere &iono)
{
#if 0
    auto it = m_store.find(sv);

    if (it == m_store.end())
        // initialize SV, if not present in store
        m_store[sv].emplace(iono.getDateOfIssue(), iono);
    else
        // FIXME: warn if there's already an entry at this time!
        it->second[iono.getDateOfIssue()] = iono;
#endif
    m_store[sv][iono.getDateOfIssue()] = iono;
    //std::cout << "exists" << std::endl;
}

std::map<DateTime, Ionosphere> IonosphereStore::getItemsBySv(const SvID &sv)
{
    return m_store[sv];
}

/**
 * @brief IonosphereStore::getIonosphere Get the Ionosphere object.
 * @param sv The SvID.
 * @return Pointer to Ionosphere for SV.
 */
Ionosphere IonosphereStore::getIonosphere(const SvID &sv, const DateTime &datetime)
{
#if 0
    auto it = m_store.find(sv);

    if (it != m_store.end())
        return it->second;
#endif

    return m_store[sv][datetime];

    //assert(false); // who called this before adding the data?!

 //   return NULL;
}

} // namespace bnav
