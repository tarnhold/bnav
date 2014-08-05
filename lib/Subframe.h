#ifndef SUBFRAME_H
#define SUBFRAME_H

#include "NavBits.h"

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

public:
    Subframe();
    Subframe(const uint32_t tow, const NavBits<300> &bits, const bool isGeo = false);

    void setBits(const NavBits<300> &bits, const bool isGeo);
    NavBits<300> getBits() const;
    void setTOW(const uint32_t tow);
    uint32_t getTOW() const;

    uint32_t getSOW() const;
    uint32_t getFrameID() const;
    uint32_t getPageNum() const;

    bool checkAndFixParityWordOne();
    bool checkAndFixParityAll();

    bool operator==(const Subframe &rhs);

private:
    void initialize();
    bool isPreambleOk() const;
    void parseSOW();
    void parseFrameID();
    void parsePageNumD1();
    void parsePageNumD2();
};

} // namespace bnav

#endif // SUBFRAME_H
