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
 * @brief IonosphereStore::addSvID Add Ionosphere to the storage.
 *
 * Determines the type of storage by SvID.
 *
 * @param sv The SvID.
 */
void IonosphereStore::addSvID(const SvID &sv)
{
    //std::pair
    m_store[sv][0] = Ionosphere();
    //m_store.emplace(sv, std::pair<uint32_t, Ionosphere*>(0, new Ionosphere()));
    //m_store.emplace(sv, );
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
    auto it = m_store.find(sv);

    // initialize SV, if not present in store
    if (it == m_store.end())
        addSvID(sv);

    it = m_store.find(sv);
    // ensure it got added
    assert(it != m_store.end());

    // FIXME: warn if there's already an entry at this time!
    it->second[iono.getSOW()] = iono;

    //std::cout << "exists" << std::endl;
}

/**
 * @brief IonosphereStore::getIonosphere Get the Ionosphere object.
 * @param sv The SvID.
 * @return Pointer to Ionosphere for SV.
 */
#if 0
Ionosphere* IonosphereStore::getIonosphere(const SvID /* &sv */)
{

    auto it = m_store.find(sv);

    if (it != m_store.end())
        return it->second;

    assert(false); // who called this before adding the data?!

    return NULL;
}
#endif

} // namespace bnav
