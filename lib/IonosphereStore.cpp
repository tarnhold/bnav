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
 * @brief IonosphereStore::addIonosphere Add a Ionosphere to the storage.
 *
 * Automatically adds a new storage object, if a given SvID is not known.
 *
 * @param sv The SvID.
 * @param iono The Ionosphere object.
 */
void IonosphereStore::addIonosphere(const SvID &sv, const Ionosphere &iono)
{
    m_store[sv][iono.getDateOfIssue()] = iono;
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

boost::optional< std::map<DateTime, Ionosphere> > IonosphereStore::getItemsBySv(const SvID &sv) const
{
    boost::optional< std::map<DateTime, Ionosphere> > items;

    const auto it = m_store.find(sv);
    if (it != m_store.end())
        items = it->second;

    return items;
}

bool IonosphereStore::hasDataForSv(const SvID &sv) const
{
    const auto it = m_store.find(sv);
    // sv doesn't exist in store
    if (it == m_store.end())
        return false;

    return !it->second.empty();
}

/**
 * @brief IonosphereStore::getIonosphere Get an Ionosphere object.
 * @param sv The SvID.
 * @param datetime Date of specific Ionosphere.
 * @return Pointer to Ionosphere for SV.
 */
boost::optional<Ionosphere> IonosphereStore::getIonosphere(const SvID &sv, const DateTime &datetime) const
{
    boost::optional<Ionosphere> ion;
    std::map<DateTime, Ionosphere> dateion;

    // find sv
    const auto dateit = m_store.find(sv);
    if (dateit != m_store.end())
        dateion = dateit->second;

    // find date
    const auto it = dateion.find(datetime);
    if (it != dateion.end())
        ion = it->second;

    return ion;
}

void IonosphereStore::dumpStoreStatistics(const std::string &name) const
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
void IonosphereStore::dumpGridAvailability(const SvID &sv) const
{
    std::cout << "Grid availability" << std::endl;

    // find sv
    const auto svit = m_store.find(sv);
    // sv is not in store
    if (svit == m_store.end())
        return;

    const auto svitems = svit->second;

    if (svitems.empty())
    {
        std::cout << "No data for SV: " << sv.getPRN() << std::endl;
        return;
    }
    else
    {
        std::cout << "Total map count: " << svitems.size() << std::endl;
    }

    // set same grid dimension
    const IonoGridDimension dim = svitems.begin()->second.getGridDimension();
    const DateTime dtref(TimeSystem::BDT, 0, 0); // just an arbitrary date
    Ionosphere ionoref;
    ionoref.setGridDimension(dim);
    ionoref.setDateOfIssue(dtref);
    // get grid and reuse tecu as counter ;)
    std::vector<IonoGridInfo> igpref = ionoref.getGrid();

    // zero initialize every single IGP
    for (auto it = igpref.begin(); it != igpref.end(); ++it)
        it->setVerticalDelay_TECU(0);

    // loop through each ionospheric model for SV in store
    for (const auto elem : svitems)
    {
        const std::vector<IonoGridInfo> igp = elem.second.getGrid();
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
