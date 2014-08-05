#include "SvID.h"

#include <cassert>

namespace bnav
{

SvID::SvID()
    : m_prn(0)
{
}

SvID::SvID(const int prn)
{
    setPRN(prn);
}

void SvID::setPRN(const int prn)
{
    assert(prn > 0 && prn <= BDS_MAX_PRN);

    m_prn = prn;
}

int SvID::getPRN() const
{
    return m_prn;
}

bool SvID::isGeo() const
{
    return m_prn <= BDS_MAX_PRN_GEO;
}

} // namespace bnav
