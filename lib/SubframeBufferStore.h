#ifndef SUBFRAMEBUFFERSTORE_H
#define SUBFRAMEBUFFERSTORE_H

#include "Subframe.h"
#include "SubframeBuffer.h"
#include "SvID.h"

#include <map>

namespace bnav
{

class SubframeBufferStore
{
    std::map<SvID, SubframeBuffer*> m_store;

public:
    SubframeBufferStore();
    ~SubframeBufferStore();

    void addSubframe(const SvID &sv, const Subframe &sf);

    SubframeBuffer* getSubframeBuffer(const SvID &sv);

    bool hasIncompleteData() const;

private:
    void addSvID(const SvID &sv);
};

} // namespace bnav

#endif // SUBFRAMEBUFFERSTORE_H
