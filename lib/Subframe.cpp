#include "Subframe.h"

#include "BeiDou.h"
#include "NavBitsECC.h"

#include <cassert>
#include <iostream>

// check parity for one word, boilerplate code. we can only use a macro here,
// because bitset expects a fixed value at compile time
#define CHECK_PARITY_FOR_WORD(pos) \
    ecc = NavBitsECCWord<30>(m_bits.getLeft<pos, 30>()); \
    if (ecc.isModified()) \
    { \
        std::cout << "fixed at " << pos << ", 30" << std::endl; \
        m_bits.setLeft(pos, ecc.getBits()); \
        m_ParityModifiedCount += ecc.getModifiedCount(); \
    }

namespace bnav
{

/*
 * References:
 *
 * [1] BDS-SIS-ICD 2.0, 2013-12
 */

Subframe::Subframe()
    : m_bits()
    , m_datetime()
    , m_sow(0)
    , m_frameID(0)
    , m_pageNum(0)
    , m_isGeo(false)
    , m_isParityFixed(false)
    , m_ParityModifiedCount(0)
    , m_isInitialized(false)
{
}

Subframe::Subframe(const SvID &sv, const DateTime date, const NavBits<300> &bits)
    : m_bits(bits)
    , m_datetime(date)
    , m_sow(0)
    , m_frameID(0)
    , m_pageNum(0)
    , m_isGeo(sv.isGeo())
    , m_isParityFixed(false)
    , m_ParityModifiedCount(0)
    , m_isInitialized(false)
{
    initialize();
}

/**
 * @brief Subframe::initialize
 *
 * Decoding procedure:
 * 1. Check Preamble
 * 2. Check and fix all parities. Unlike the ICD, all messages are made up of
 *    words with 30 bits. This means that there are no special pages like D2
 *    subframe 4 with 72 parity bits at the end.
 * 3. Decode FraID from first word.
 * 4. Decode Pnum.
 */
void Subframe::initialize()
{
    if (!isPreambleOk())
        std::cerr << "Wrong preamble: " << m_bits << std::endl;

    // fix all remaining words
    // other than the ICD says, there are no blocks like D2 subframe 4, which
    // has 72 parity bits at the end of the message. Those pages are as all
    // other, 30+30+30...
    checkAndFixParities();

    // read basic info from NavBits
    parseFrameID();
    parseSOW();

    // no parity check before, because of special pages...
    if (m_isGeo)
        parsePageNumD2();
    else
        parsePageNumD1();

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

void Subframe::setDateTime(const DateTime &date)
{
    m_datetime = date;
}

void Subframe::setSvID(const SvID &sv)
{
    m_isGeo = sv.isGeo();
}

DateTime Subframe::getDateTime() const
{
    return m_datetime;
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

std::size_t Subframe::getParityModifiedCount() const
{
    return m_ParityModifiedCount;
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

bool Subframe::checkAndFixParities()
{
    // second 15 bits of word one need to be checked
    // first 15 bits are preamble and 4 bit reserved
    NavBitsECCWord<15> ecc1(m_bits.getLeft<15, 15>());
    if (ecc1.isModified())
    {
        // SOW is not this helpful here, because we have an offset between SOW
        // and TOW from sbf file, but better than nothing
        std::cout << "Parity fixed for SOW: " << m_datetime.getSOW() << std::endl;
        m_bits.setLeft(15, ecc1.getBits());
        m_ParityModifiedCount += ecc1.getModifiedCount();
    }

    // fix remaining words
    NavBitsECCWord<30> ecc(0);
    CHECK_PARITY_FOR_WORD(30);
    CHECK_PARITY_FOR_WORD(60);
    CHECK_PARITY_FOR_WORD(90);
    CHECK_PARITY_FOR_WORD(120);
    CHECK_PARITY_FOR_WORD(150);
    CHECK_PARITY_FOR_WORD(180);
    CHECK_PARITY_FOR_WORD(210);
    CHECK_PARITY_FOR_WORD(240);
    CHECK_PARITY_FOR_WORD(270);

    m_isParityFixed= true;
    return m_isParityFixed;
}

/**
 * @brief operator == Check for equality of two Subframes.
 * @return true if TOW and NavBits are equal, false if unequal.
 */
bool Subframe::operator==(const Subframe &rhs)
{
    // check only Timestamp and NavBits, if they are equal
    // assume it's the same Subframe.
    return (m_datetime == rhs.getDateTime()) && (m_bits == rhs.getBits());
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
