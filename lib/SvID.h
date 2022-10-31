#ifndef SVID_H
#define SVID_H

#include <cstdint>

namespace bnav
{

/**
 * @brief The SvID class
 *
 * Maintain satellite IDs and their unterlying type (GEO, MEO, IGSO).
 */
class SvID
{
    uint32_t m_prn;

public:
    SvID();
    SvID(const uint32_t prn);

    void setPRN(const uint32_t prn);
    uint32_t getPRN() const;

    bool operator<(const SvID &rhs) const;
    bool operator==(const SvID &rhs) const;
    bool operator!=(const SvID &rhs) const;

    bool isGeo() const;
    // maybe implement:
    // bool isMEO() const;
    // bool isIGSO() const;
};

} // namespace bnav

#endif // SVID_H
