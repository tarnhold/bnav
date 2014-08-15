#include "SvID.h"

#include <cassert>

namespace bnav
{

SvID::SvID()
    : m_prn(0)
{
}

SvID::SvID(const uint32_t prn)
{
    setPRN(prn);
}

void SvID::setPRN(const uint32_t prn)
{
    assert(prn > 0 && prn <= BDS_MAX_PRN);

    m_prn = prn;
}

uint32_t SvID::getPRN() const
{
    return m_prn;
}

bool SvID::operator <(const SvID &rhs) const
{
    return m_prn < rhs.getPRN();
}

bool SvID::isGeo() const
{
    return (m_prn > 0) && (m_prn <= BDS_MAX_PRN_GEO);
}

} // namespace bnav
