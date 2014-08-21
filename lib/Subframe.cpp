#include "Subframe.h"

#include "BeiDou.h"
#include "NavBitsECC.h"

#include <cassert>
#include <iostream>

namespace bnav
{

/*
 * References:
 *
 * [1] BDS-SIS-ICD 2.0, 2013-12
 */

Subframe::Subframe()
    : m_tow(0)
    , m_sow(0)
    , m_frameID(0)
    , m_pageNum(0)
    , m_isGeo(false)
    , m_isParityWordOneFixed(false)
    , m_isParityAllFixed(false)
    , m_isInitialized(false)
{
}

Subframe::Subframe(const SvID &sv, const uint32_t tow, const NavBits<300> &bits)
    : m_tow(tow)
    , m_sow(0)
    , m_frameID(0)
    , m_pageNum(0)
    , m_isGeo(sv.isGeo())
    , m_isParityWordOneFixed(false)
    , m_isParityAllFixed(false)
    , m_isInitialized(false)
{
    m_bits = bits;

    initialize();
}

/**
 * @brief Subframe::initialize
 *
 * Decoding procedure:
 * 1. Check Preabmle
 * 2. Check and fix parity for second half of the first word.
 * 3. Decode FraID from first word.
 * 4. Decode Pnum without parity check from second word.
 *
 * TODO (see notes from 17.07.2014 PageNum):
 * 5. After Pnum has been read, do complete parity check of the whole message.
 *      (maybe delay this until all pages are accumulated inside SubframeBuffer)
 */
void Subframe::initialize()
{
    if (!isPreambleOk())
        std::cerr << "Wrong preamble: " << m_bits << std::endl;

    // ensure our FraID is safe
    checkAndFixParityWordOne();

    // read basic info from NavBits
    parseFrameID();
    parseSOW();

    // no parity check before, because of special pages...
    if (m_isGeo)
        parsePageNumD2();
    else
        parsePageNumD1();

    // fix all remaining words
    checkAndFixParityAll();

    m_isInitialized = true;
}

void Subframe::setBits(const NavBits<300> &bits)
{
    m_bits = bits;
}

NavBits<300> Subframe::getBits() const
{
    return m_bits;
}

void Subframe::setTOW(const uint32_t tow)
{
    m_tow = tow;
}

void Subframe::setSvID(const SvID &sv)
{
    m_isGeo = sv.isGeo();
}

uint32_t Subframe::getTOW() const
{
    return m_tow;
}

uint32_t Subframe::getSOW() const
{
    assert(m_isInitialized);
    return m_sow;
}

uint32_t Subframe::getFrameID() const
{
    assert(m_isInitialized);
    return m_frameID;
}

uint32_t Subframe::getPageNum() const
{
    assert(m_isInitialized);
    return m_pageNum;
}

#if 0
/**
 * @brief Subframe::forcePageNum Force Subframe to be a specific Pnum.
 *
 * Could be used, if Subframe is suspicious to have a parity error, which
 * is not currently fixed (because Pnum is read plain). So we force Pnum
 * and try to fix all parities after that.
 *
 * @param pnum Index of page number.
 */
void Subframe::forcePageNum(const uint32_t pnum)
{
    m_pageNum = pnum;
}
#endif

bool Subframe::checkAndFixParityWordOne()
{
    // second 15 bits of word one need to be checked
    // first 15 bits are preamble and 4 bit reserved
    NavBits<15> wordone = m_bits.getLeft<15, 15>();

    NavBitsECC<15> ecc(wordone);
    ecc.checkAndFixAll();
    if (ecc.isModified())
    {
        std::cerr << "Parity fixed for TOW: " << m_tow << std::endl;
        wordone = ecc.getBits();
// FIXME currently only checked, not further used (corrected data should be
// saved back into NavBits
    }

    // TODO
    m_isParityWordOneFixed = false;
    return m_isParityWordOneFixed;
}

bool Subframe::checkAndFixParityAll()
{
#if 0
    if (m_isGeo && m_frameID == 5)
    {
        if ((m_pageNum > 0 && m_pageNum <= 13)
                && (m_pageNum >= 35 && m_pageNum <= 73))
        {
            NavBits<270> remaining = m_bits.getLeft<30, 270>();
            NavBitsECC<15> ecc(remaining);
            ecc.checkAndFixAll();
            if (ecc.isModified())
            {
                std::cerr << "Parity fixed for TOW: " << m_tow << std::endl;
                remaining = ecc.getBits();
        // FIXME currently only checked, not further used (corrected data should be
        // saved back into NavBits
            }

        }
    }
#endif

    // TODO remaining parities are ignored at the moment
    m_isParityAllFixed = false;
    return m_isParityAllFixed;
}

/**
 * @brief operator == Check for equality of two Subframes.
 * @return true if TOW and NavBits are equal, false if unequal.
 */
bool Subframe::operator==(const Subframe &rhs)
{
    // check only TOW and NavBits, if they are equal
    // assume it's the same Subframe.
    return (m_tow == rhs.getTOW()) && (m_bits == rhs.getBits());
}

/**
 * @brief Subframe::isPreambleOk Check if message starts with the BeiDou preamble.
 * @return true if preamble is "11100010010", false if not
 */
bool Subframe::isPreambleOk() const
{
    NavBits<11> pre = m_bits.getLeft<0, 11>();

    // (11100010010)bin is (1810)dec
    return pre.to_ulong() == BDS_PREABMLE;
}

/**
 * @brief Subframe::parseFrameID Read FraID from message bits.
 *
 * Reference: [1] 5.2.4.2 Subframe identification
 */
void Subframe::parseFrameID()
{
    NavBits<3> fraID = m_bits.getLeft<15, 3>();
//    std::cout << m_bits.getLeft<15, 3>() << " fraID: " << fraID << " : " << fraID.to_ulong() << std::endl;

    m_frameID = fraID.to_ulong();

    // FraIDs between 1 and 5 are valid
    assert(m_frameID > 0 && m_frameID < 6);
}

/**
 * @brief Subframe::parseSOW Read SOW from NavBits.
 *
 * Reference: [1] 5.2.3 D1 NAV Message Detailed Structure
 */
void Subframe::parseSOW()
{
    NavBits<8> sow1 = m_bits.getLeft<18, 8>();
    NavBits<12> sow2 = m_bits.getLeft<30, 12>();

    // merge both sow parts
    NavBits<20> sow(sow1);
    sow <<= 12;
    sow ^= sow2;

    m_sow = sow.to_ulong();

    // SOW between 0 and 604800 are valid
    assert(m_sow >= 0 && m_sow < 60*60*24*7);
}

/**
 * @brief Subframe::parsePageNum Read Pnum from NavBits for D1.
 *
 * Reference: [1] 5.2.3 D1 Nav Message Detailed Structure
 */
void Subframe::parsePageNumD1()
{
    // parse frameID, if not already happened
    if (m_frameID == 0)
        parseFrameID();

    // for D1, only frame 4 and 5 have Pnum
    if (m_frameID > 3)
    {
        // Pnum
        NavBits<7> pnum = m_bits.getLeft<43, 7>();
        m_pageNum = pnum.to_ulong();
        assert(m_pageNum > 0 && m_pageNum <= 24);
    }

//    std::cout << "D1: frame: " << m_frameID << " page: " << m_pageNum << std::endl;
}

/**
 * @brief Subframe::parsePageNumD2 Read Pnum from NavBits for D2.
 *
 * Reference: [1] 5.3.2 D2 NAV Message Detailed Structure
 */
void Subframe::parsePageNumD2()
{
    // parse frameID, if not already happened
    if (m_frameID == 0)
        parseFrameID();

    if (m_frameID == 1)
    {
        // Pnum1
        NavBits<4> pnum = m_bits.getLeft<42, 4>();
        m_pageNum = pnum.to_ulong();
        assert(m_pageNum > 0 && m_pageNum <= 10);
    }
    // frameID 3 and 4 have no Pnum, they use that from FrameID 2
    // as those frames only contain integrity information, which is
    // not handled by this program, we ignore this detail.
    else if (m_frameID == 2)
    {
        // Pnum2
        NavBits<4> pnum = m_bits.getLeft<43, 4>();
        m_pageNum = pnum.to_ulong();
        assert(m_pageNum > 0 && m_pageNum <= 6);
    }
    else if (m_frameID == 5)
    {
        // Pnum
        NavBits<7> pnum = m_bits.getLeft<43, 7>();
        m_pageNum = pnum.to_ulong();
        assert(m_pageNum > 0 && m_pageNum <= 120);
    }

//    std::cout << "D2: frame: " << m_frameID << " page: " << m_pageNum << std::endl;
}

} // namespace bnav
