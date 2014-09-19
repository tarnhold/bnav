#include "IonosphereGridInfo.h"

#include "BeiDou.h"

namespace
{

// [1] 5.3.3.8.2 Grid Ionospheric Vertical Error Index (GIVEI)
constexpr double GIVEI_LOOKUP_TABLE[] { 0.3, 0.6, 0.9, 1.2,
                                      1.5, 1.8, 2.1, 2.4,
                                      2.7, 3.0, 3.6, 4.5,
                                      6.0, 9.0, 15.0, 45.0};

uint32_t lcl_convertMeterToTECU(const double value, const double freq)
{
    // 100 meters are 6047 TECU for B1
    assert(value >= 0 && value < 100);
    // 1 TECU is 1.0e16 el/m2
    // IONEX refers to 0.1 TECU, so we need to multiply * 10
    return static_cast<uint32_t>(std::lround((value * (freq * freq) / 40.3) / 1.0e16 * 10));
}

/**
 * @brief lcl_diffTECUValues Difference between two TECU values.
 * @param t1 TECU 1
 * @param t2 TECU 2
 * @return Invalid 9999, if one of both is invalid, otherwise absolute difference.
 */
uint32_t lcl_diffTECUValues(const uint32_t t1, const uint32_t t2)
{
    // comprises two cases:
    // 1. one of both is not available
    // 2. both are not available
    if (t1 == 9999 || t2 == 9999)
        return 9999;

    if (t2 > t1)
        return t2 - t1;
    else
        return t1 - t2;
}

}

namespace bnav
{

/**
 * @brief The IonoGridInfo class
 *
 * Manages information of one ionospheric grid point (Ion).
 *
 * [1]. 5.3.3.8 Ionospheric Grid Information (Ion)
 */
IonoGridInfo::IonoGridInfo()
    : m_dtTECU(UINT32_MAX)
    , m_giveTECU(UINT32_MAX)
    , m_isValid(false)
{
}

IonoGridInfo::IonoGridInfo(const uint32_t vertdelay, const uint32_t rms)
    : IonoGridInfo()
{
    load(vertdelay, rms);
}

IonoGridInfo::IonoGridInfo(const NavBits<13> &bits)
    : IonoGridInfo()
{
    load(bits);
}

void IonoGridInfo::load(const uint32_t vertdelay, const uint32_t rms)
{
    m_isValid = true;
    m_dtTECU = vertdelay;
    m_giveTECU = rms;
}

void IonoGridInfo::load(const NavBits<13> &bits)
{
    // [1] 5.3.2 D2 NAV Message Detailed structure, p. 50
    // first 9 bits are dt
    NavBits<9> bitsdt { bits.getLeft<0, 9>() };
    // last 4 bits are givei
    NavBits<4> bitsgivei { bits.getLeft<9, 4>() };

    loadVerticalDelay(bitsdt);
    loadGivei(bitsgivei);

    // set state to valid
    m_isValid = true;
}

void IonoGridInfo::loadFromMeter(const double vertdelay, const double rms)
{
    load(lcl_convertMeterToTECU(vertdelay, BDS_B1I_FREQ), lcl_convertMeterToTECU(rms, BDS_B1I_FREQ));
}

void IonoGridInfo::loadVerticalDelay(const NavBits<9> &bits)
{
    uint32_t dtraw { bits.to_ulong() };
    // maximum value is 63.875m, which is 511 when scaled by 0.125
    assert(dtraw <= 511);
    // convert to TECU
    if (dtraw >= 510)
        m_dtTECU =  9999;
    else
    {
        // metric
        double dt { dtraw * 0.125 };
        m_dtTECU = lcl_convertMeterToTECU(dt, BDS_B1I_FREQ);
    }
}

void IonoGridInfo::loadGivei(const NavBits<4> &bits)
{
    uint32_t givei { bits.to_ulong() };
    assert(givei <= 15);

    // according to the ICD there are no invalid values, but invalid dt values
    // get GIVEI 15, so set this to invalid, too
    if (givei == 15)
        m_giveTECU = 9999;
    else
        m_giveTECU = lcl_convertMeterToTECU(GIVEI_LOOKUP_TABLE[givei], BDS_B1I_FREQ);
}

void IonoGridInfo::setVerticalDelay_TECU(const uint32_t tec)
{
    assert(tec < 10000);
    m_dtTECU = tec;
    m_isValid = true;
}

/**
 * @brief IonoGridInfo::getVerticalDelay_TECU Return ionospheric delay as 0.1 TECU.
 * @return TECU value, 9999 if not available.
 */
uint32_t IonoGridInfo::getVerticalDelay_TECU() const
{
    assert(m_isValid);
    return m_dtTECU;
}

/**
 * @brief IonoGridInfo::getGive_TECU Return delay correction accuracy as 0.1 TECU.
 * @return TECU value.
 */
uint32_t IonoGridInfo::getGive_TECU() const
{
    assert(m_isValid);
    return m_giveTECU;
}

bool IonoGridInfo::operator==(const IonoGridInfo &rhs) const
{
    assert(m_isValid);
    return rhs.getVerticalDelay_TECU() == getVerticalDelay_TECU()
           && rhs.getGive_TECU() == getGive_TECU();
}

IonoGridInfo IonoGridInfo::operator-(const IonoGridInfo &rhs) const
{
    uint32_t dvdel { lcl_diffTECUValues(getVerticalDelay_TECU(), rhs.getVerticalDelay_TECU()) };
    uint32_t dgive { lcl_diffTECUValues(getGive_TECU(), rhs.getGive_TECU()) };

    return IonoGridInfo(dvdel, dgive);
}

} // namespace bnav
