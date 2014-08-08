#ifndef SUBFRAMEBUFFER_H
#define SUBFRAMEBUFFER_H

#include "Subframe.h"

#include <vector>

namespace bnav
{

using SubframeVector = std::vector< std::vector< Subframe > >;

class SubframeBuffer
{
    // save: frame< pages >
    SubframeVector m_buffer;

    uint32_t m_lastsow;

public:
    SubframeBuffer();

    void addSubframe(const Subframe &sf);

    bool isEphemerisComplete() const;
    bool isIntegrityComplete() const;
    bool isAlmanacComplete() const;

    SubframeVector flushEphemerisData();
    //SubframeVector flushIntegrityData();
    SubframeVector flushAlmanacData();

    void clearEphemerisData();
    void clearIntegrityData();
    void clearAlmanacData();
};

} // namespace bnav

#endif // SUBFRAMEBUFFER_H
