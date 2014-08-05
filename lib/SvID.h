#ifndef SVID_H
#define SVID_H

namespace bnav
{

static const int BDS_MAX_PRN = 37;
static const int BDS_MAX_PRN_GEO = 5;

/**
 * @brief The SvID class
 *
 * Maintain satellite IDs and their unterlying type (GEO, MEO, IGSO).
 */
class SvID
{
    int m_prn;

public:
    SvID();
    SvID(const int prn);

    void setPRN(const int prn);
    int getPRN() const;

    bool isGeo() const;
    // maybe implement:
    // bool isMEO() const;
    // bool isIGSO() const;
};

} // namespace bnav

#endif // SVID_H
