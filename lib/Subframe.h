#ifndef SUBFRAME_H
#define SUBFRAME_H

#include "NavBits.h"
#include "SvID.h"

namespace bnav
{

/**
 * @brief The Subframe class
 *
 * Forms a subframe. Does decoding of FraID, Pnum and SOW.
 */
class Subframe
{
    NavBits<300> m_bits;
    uint32_t m_tow;
    uint32_t m_sow;
    uint32_t m_frameID;
    uint32_t m_pageNum;
    bool m_isGeo;

    bool m_isParityWordOneFixed;
    bool m_isParityAllFixed;
    bool m_isInitialized;

public:
    Subframe();
    Subframe(const SvID &sv, const uint32_t tow, const NavBits<300> &bits);

    void setBits(const NavBits<300> &bits);
    NavBits<300> getBits() const;
    void setTOW(const uint32_t tow);
    uint32_t getTOW() const;

    void setSvID(const SvID &sv);

    void initialize();

    uint32_t getSOW() const;
    uint32_t getFrameID() const;
    uint32_t getPageNum() const;

#if 0
    void forcePageNum(const uint32_t pnum);
#endif

    bool checkAndFixParityWordOne();
    bool checkAndFixParityAll();

    bool operator==(const Subframe &rhs);

private:
    bool isPreambleOk() const;
    void parseSOW();
    void parseFrameID();
    void parsePageNumD1();
    void parsePageNumD2();
};

} // namespace bnav

#endif // SUBFRAME_H
