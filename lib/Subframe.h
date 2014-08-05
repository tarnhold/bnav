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
    int m_tow;
    int m_sow;
    int m_frameID;
    int m_pageNum;
    bool m_isGeo;

    bool m_isParityWordOneFixed;
    bool m_isParityAllFixed;

public:
    Subframe();
    Subframe(const int tow, const NavBits<300> &bits, const bool isGeo = false);

    void setBits(const NavBits<300> &bits, const bool isGeo);
    NavBits<300> getBits() const;
    void setTOW(const int tow);
    int getTOW() const;

    int getSOW() const;
    int getFrameID() const;
    int getPageNum() const;

    bool checkAndFixParityWordOne();
    bool fixParityAll();

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
