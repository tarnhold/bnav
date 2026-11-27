#include "SubframeBufferStore.h"

#include <cassert>
#include <memory>
#include <utility>

namespace bnav
{

/**
 * @brief SubframeBufferStore::SubframeBufferStore Storage for SubframeBuffer
 * object.
 *
 * Used to manage multiple SubframeBuffer objects for multiple SVs.
 */
SubframeBufferStore::SubframeBufferStore()
    : m_store()
{
}

/**
 * @brief SubframeBufferStore::addSvID Add SubframeBuffer to the storage.
 *
 * Determines the type of storage by SvID.
 *
 * @param sv The SvID.
 */
void SubframeBufferStore::addSvID(const SvID &sv)
{
    std::unique_ptr<SubframeBuffer> sfbuf = sv.isGeo()
            ? std::unique_ptr<SubframeBuffer>{std::make_unique<SubframeBufferD2>()}
            : std::unique_ptr<SubframeBuffer>{std::make_unique<SubframeBufferD1>()};

    m_store.emplace(sv, std::move(sfbuf));
}

/**
 * @brief SubframeBufferStore::addSubframe Add a subframe to the storage.
 *
 * Automatically adds a new storage object, if a given SvID is not known.
 *
 * @param sv The SvID.
 * @param sf The Subframe object.
 */
void SubframeBufferStore::addSubframe(const SvID &sv, const Subframe &sf)
{
    auto it = m_store.find(sv);

    // initialize SV, if not present in store
    if (it == m_store.end())
        addSvID(sv);

    it = m_store.find(sv);
    // ensure it got added
    assert(it != m_store.end());

    it->second->addSubframe(sf);
}

/**
 * @brief SubframeBufferStore::getSubframeBuffer Get the SubframeBuffer object.
 * @param sv The SvID.
 * @return Non-owning pointer to SubframeBuffer for SV.
 */
SubframeBuffer* SubframeBufferStore::getSubframeBuffer(const SvID &sv)
{
    const auto it = m_store.find(sv);

    if (it != m_store.end())
        return it->second.get();

    assert(false); // who called this before adding the data?!
    return nullptr;
}

/**
 * @brief SubframeBufferStore::hasIncompleteData Checks if any SubframeBuffer
 * of a SV has an incomplete data set.
 *
 * @return true, if any buffer isn't empty. false, if all buffers are empty.
 */
bool SubframeBufferStore::hasIncompleteData() const
{
    for (const auto &elem : m_store)
    {
        if (elem.second->hasIncompleteData())
            return true;
    }

    return false;
}

} // namespace bnav
