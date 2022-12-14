#ifndef SUBFRAME_H
#define SUBFRAME_H

#include "NavBits.h"
#include "SvID.h"

#include <cstdint>

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
    uint32_t m_sow;
    uint32_t m_frameID;
    uint32_t m_pageNum;
    bool m_isGeo;

    bool m_isParityFixed;
    std::size_t m_ParityModifiedCount;
    bool m_isInitialized;

public:
    Subframe();
    Subframe(const SvID &sv, const NavBits<300> &bits);

    void setBits(const NavBits<300> &bits);
    NavBits<300> getBits() const;
    std::size_t getParityModifiedCount() const;

    void setSvID(const SvID &sv);
    void setPageNum(const std::size_t pnum);

    void initialize();

    uint32_t getSOW() const;
    uint32_t getFrameID() const;
    uint32_t getPageNum() const;

    bool operator==(const Subframe &rhs);

private:
    bool isPreambleOk() const;
    bool checkAndFixParities();

    void parseSOW();
    void parseFrameID();
    void parsePageNumD1();
    void parsePageNumD2();
};

} // namespace bnav

#endif // SUBFRAME_H
