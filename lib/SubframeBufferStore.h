#ifndef SUBFRAMEBUFFERSTORE_H
#define SUBFRAMEBUFFERSTORE_H

#include "Subframe.h"
#include "SubframeBuffer.h"
#include "SvID.h"

#include <map>
#include <memory>

namespace bnav
{

class SubframeBufferStore
{
    // Owns one D1 or D2 buffer per satellite. getSubframeBuffer() returns
    // a non-owning pointer that stays valid until the store is destroyed.
    std::map<SvID, std::unique_ptr<SubframeBuffer>> m_store;

public:
    SubframeBufferStore();

    void addSubframe(const SvID &sv, const Subframe &sf);

    [[nodiscard]] SubframeBuffer* getSubframeBuffer(const SvID &sv);

    [[nodiscard]] bool hasIncompleteData() const;

private:
    void addSvID(const SvID &sv);
};

} // namespace bnav

#endif // SUBFRAMEBUFFERSTORE_H
