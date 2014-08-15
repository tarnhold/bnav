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
    std::map<SvID, SubframeBufferD1> m_store;

public:
    SubframeBufferStore();

    void addSubframe(const Subframe &sf);

    SubframeBufferD1 getSubframeBuffer(const SvID &sv);


};

} // namespace bnav

#endif // SUBFRAMEBUFFERSTORE_H
