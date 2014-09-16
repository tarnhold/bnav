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

/**
 * @brief IonosphereStore::getSvList Get a list of all SV in store.
 * @return SvIDs inside a vector.
 */
std::vector< SvID > IonosphereStore::getSvList() const
{
    std::vector< SvID > svlist {};

    for (const auto elem : m_store)
        svlist.push_back(elem.first);

    return svlist;
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

void IonosphereStore::dumpStoreStatistics(const std::string name)
{
    std::cout << "IonosphereStore statistics: " << name << std::endl;

    if (m_store.empty())
    {
        std::cout << "No items in store!" << std::endl;
        return;
    }

    for (const auto item : m_store)
    {
        std::cout << item.first.getPRN() << ": "
                  << item.second.size() << std::endl;
    }
}

/**
 * @brief IonosphereStore::dumpGridAvailability Generate a simple grid
 * availability map. Counts all elements which are not 9999.
 *
 * @param sv Satellite for which we produce a map.
 */
void IonosphereStore::dumpGridAvailability(const SvID &sv)
{
    std::cout << "Grid availability" << std::endl;

    if (m_store[sv].empty())
    {
        std::cout << "No data for SV: " << sv.getPRN() << std::endl;
        return;
    }
    else
    {
        std::cout << "Total map count: " << m_store[sv].size() << std::endl;
    }

    // set same grid dimension
    IonoGridDimension dim = m_store[sv].begin()->second.getGridDimension();
    DateTime dtref(TimeSystem::BDT, 0, 0); // just an arbitrary date
    Ionosphere ionoref;
    ionoref.setGridDimension(dim);
    ionoref.setDateOfIssue(dtref);
    // get grid and reuse tecu as counter ;)
    std::vector<IonoGridInfo> igpref = ionoref.getGrid();

    // zero initialize every single IGP
    for (auto it = igpref.begin(); it != igpref.end(); ++it)
        it->setVerticalDelay_TECU(0);

    // loop through each ionospheric model for SV in store
    for (auto elem : m_store[sv])
    {
        std::vector<IonoGridInfo> igp = elem.second.getGrid();
        std::size_t i = 0;

        for (auto igpelem : igp)
        {
            // increment if real IGP point has data
            if (igpelem.getVerticalDelay_TECU() != 9999)
            {
                igpref[i].setVerticalDelay_TECU(igpref[i].getVerticalDelay_TECU() + 1);
            }

            ++i;
        }
    }

    ionoref.setGrid(igpref);
    ionoref.dump();
}

} // namespace bnav
