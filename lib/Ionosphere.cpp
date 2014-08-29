#include "Ionosphere.h"
#include "BeiDou.h"
#include "NavBits.h"
#include "SubframeBuffer.h"

#include <cmath>
#include <math.h>
#include <iostream>

namespace
{

uint32_t lcl_convertMeterToTECU(const double value, const double freq)
{
    assert(value >= 0);
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

#if 0
/**
 * @brief lcl_calcKlobucharCorrection Calculate Klobuchar correction at position
 * (phi, lambda). This is only vertical delay, no slant!
 *
 * @param klob Klobuchar parameters.
 * @param time Local time.
 * @param phi Phi in degrees, east of Greenwich is negative.
 * @param lambda Lambda in degrees, north of equator is positive.
 * @return Vertical delay in meters.
 *
 * References:
 * [1] ICD, 5.2.4.7 Ionospheric Delay Model Parameters, pp. 25
 * [2] Ionospheric Time-Delay Algorithm for Single Frequency GPS Users,
 *     John. A. Klobuchar, 1987
 * [3] http://www.navipedia.net/index.php/Klobuchar_Ionospheric_Model
 */
double lcl_calcKlobucharCorrection(const bnav::KlobucharParam &klob, const uint32_t time, const double phi, const double lambda)
{
    assert(time < 86400);

    // convert to semicircle
    double semiphi = phi / 180.0;
    double semilambda = lambda / 180.0; // east!

//    std::cout << "phi: " << semiphi << std::endl;

    if (semiphi > 0.416)
        semiphi = 0.416;
    if (semiphi < -0.416)
        semiphi = -0.416;

    // geomagnetic latitude
    double phim = semiphi + 0.064 * std::cos((semilambda - 1.617)*M_PI);

//    std::cout << "phim: " << phim << std::endl;

    //std::cout << "timeb: " << static_cast<int32_t>(4.32 * 1.0e4 * lambda) << " + " << static_cast<int32_t>(time) << std::endl;
    int32_t time2 =  static_cast<int32_t>(4.32 * 1.0e4 * semilambda) + static_cast<int32_t>(time);
    if (time2 > 86400)
        time2 = time2 - 86400;
    if (time2 < 0)
        time2 = time2 + 86400;

//    std::cout << "time: " << time2 << std::endl;

    double amplitude = klob.alpha0 + phim * (klob.alpha1 + phim * (klob.alpha2 + phim * klob.alpha3));
    double period = klob.beta0 + phim * (klob.beta1 + phim * (klob.beta2 + phim * klob.beta3));

    if (amplitude < 0.0)
        amplitude = 0.0;
    if (period < 72000.0)
        period = 72000.0;
    if (period >= 172800.0)
        period = 172800.0;

    const double x = 2 * M_PI * (time2 - 50400) / period;

//    std::cout << "x: " << x << std::endl;

    double tiono;

    // ignore slant factor F, because we want vertical delay
    if (std::fabs(x) < M_PI / 2.0)
        tiono = 5.0e-9 + amplitude * (1 - std::pow(x, 2)/2 + std::pow(x, 4)/24 );
    else
        tiono = 5.0e-9;

    // we have time atm
    return tiono * bnav::SPEED_OF_LIGHT;
}
#endif

/**
 * @brief lcl_calcKlobucharCorrectionBDS Calculate Klobuchar correction at position
 * (phi, lambda). This is only vertical delay, no slant!
 *
 * @param klob Klobuchar parameters.
 * @param time Local time.
 * @param phi Phi in degrees, east of Greenwich is negative.
 * @param lambda Lambda in degrees, north of equator is positive.
 * @return Vertical delay in meters.
 *
 * References:
 * [1] ICD, 5.2.4.7 Ionospheric Delay Model Parameters, pp. 25
 */
double lcl_calcKlobucharCorrectionBDS(const bnav::KlobucharParam &klob, const uint32_t time, const double phi, const double lambda)
{
    assert(time < 86400);

    // convert to semicircle
    double semiphi = phi / 180.0;
    double semilambda = lambda / 180.0;

//    std::cout << "phi: " << semiphi << std::endl;

    // chinese don't do geomagnetic latitude
    double phim = semiphi;

//    std::cout << "phim: " << phim << std::endl;

    //std::cout << "timeb: " << static_cast<int32_t>(4.32 * 1.0e4 * lambda) << " + " << static_cast<int32_t>(time) << std::endl;
    int32_t localtime =  static_cast<int32_t>(4.32e4 * semilambda) + static_cast<int32_t>(time);

    if (localtime > 86400)
        localtime = localtime - 86400;
    if (localtime < 0)
        localtime = localtime + 86400;

//    std::cout << "time: " << time2 << std::endl;

    double amplitude = klob.alpha0 + phim * (klob.alpha1 + phim * (klob.alpha2 + phim * klob.alpha3));
    double period = klob.beta0 + phim * (klob.beta1 + phim * (klob.beta2 + phim * klob.beta3));

    if (amplitude < 0.0)
        amplitude = 0.0;
    if (period < 72000.0)
        period = 72000.0;
    if (period >= 172800.0)
        period = 172800.0;

    const double x = 2 * M_PI * (localtime - 50400) / period;

//    std::cout << "x: " << x << std::endl;

    double tiono;

    // ignore slant factor F, because we want vertical delay
    if (std::fabs(localtime - 50400) < period / 4.0)
        tiono = 5.0e-9 + amplitude * std::cos(x);
    else
        tiono = 5.0e-9;

    // we have time atm
    return tiono * bnav::SPEED_OF_LIGHT;
}

// [1] 5.3.3.8.2 Grid Ionospheric Vertical Error Index (GIVEI)
const double GIVEI_LOOKUP_TABLE[] = { 0.3, 0.6, 0.9, 1.2,
                                      1.5, 1.8, 2.1, 2.4,
                                      2.7, 3.0, 3.6, 4.5,
                                      6.0, 9.0, 15.0, 45.0};

/**
 * Parse an 13 bit ionospheric info element (dt and givei), which is splitted by
 * parity information. This helper is used to collect msb and lsb blocks and
 * to merge them.
 */
template <std::size_t msb, std::size_t msb_len,
          std::size_t lsb, std::size_t lsb_len>
bnav::NavBits<13> lcl_parsePageIon(const bnav::NavBits<300> &bits)
{
    // msb bits have to be left of lsb bits
    assert(msb < lsb);
    assert(msb + msb_len < lsb);
    // we need this method to skip parities, check if we skip 8 bits
    assert(msb + msb_len + 8 == lsb);
    // one Ion element is 13 bits long
    assert(msb_len + lsb_len == 13);

    bnav::NavBits<13> iono = bits.getLeft<msb, msb_len>();
    iono <<= lsb_len;
    // add lsb part to iono
    bnav::NavBits<lsb_len> lsbits = bits.getLeft<lsb, lsb_len>();
    iono ^= lsbits;

    return iono;
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

IonoGridInfo::IonoGridInfo(const NavBits<13> &bits)
    : IonoGridInfo()
{
    load(bits);
}

IonoGridInfo::IonoGridInfo(const uint32_t vertdelay, const uint32_t rms)
    : IonoGridInfo()
{
    m_isValid = true;
    m_dtTECU = vertdelay;
    m_giveTECU = rms;
}

void IonoGridInfo::load(const NavBits<13> &bits)
{
    // [1] 5.3.2 D2 NAV Message Detailed structure, p. 50
    // first 9 bits are dt
    NavBits<9> bitsdt = bits.getLeft<0, 9>();
    // last 4 bits are givei
    NavBits<4> bitsgivei = bits.getLeft<9, 4>();

    loadVerticalDelay(bitsdt);
    loadGivei(bitsgivei);

    // set state to valid
    m_isValid = true;
}

void IonoGridInfo::loadVerticalDelay(const NavBits<9> &bits)
{
    uint32_t dtraw = bits.to_ulong();
    // maximum value is 63.875m, which is 511 when scaled by 0.125
    assert(dtraw >= 0 && dtraw <= 511);
    // convert to TECU
    if (dtraw >= 510)
        m_dtTECU =  9999;
    else
    {
        // metric
        double dt = dtraw * 0.125;
        m_dtTECU = lcl_convertMeterToTECU(dt, BDS_B1I_FREQ);
    }
}

void IonoGridInfo::loadGivei(const NavBits<4> &bits)
{
    uint32_t givei = bits.to_ulong();
    assert(givei >= 0 && givei <= 15);

    // according to the ICD there are no invalid values, but invalid dt values
    // get GIVEI 15, so set this to invalid, too
    if (givei == 15)
        m_giveTECU = 9999;
    else
        m_giveTECU = lcl_convertMeterToTECU(GIVEI_LOOKUP_TABLE[givei], BDS_B1I_FREQ);
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
    uint32_t dvdel = lcl_diffTECUValues(getVerticalDelay_TECU(), rhs.getVerticalDelay_TECU());
    uint32_t dgive = lcl_diffTECUValues(getGive_TECU(), rhs.getGive_TECU());

    return IonoGridInfo(dvdel, dgive);
}

/**
 * @brief The Ionosphere class
 *
 * Stores IGP data into a single row vector.
 */
Ionosphere::Ionosphere()
    : m_sow(0)
{
}

Ionosphere::Ionosphere(const SubframeBufferParam &sfbuf)
{
    load(sfbuf);
}

Ionosphere::Ionosphere(const KlobucharParam &klob, const uint32_t time)
{
    load(klob, time);
}

void Ionosphere::load(const KlobucharParam &klob, const uint32_t sow)
{
    assert(sow <= SECONDS_OF_A_WEEK);
    // strictly speaking we need GPST here, but 16s,... be lazy
    // we need only time of day, not full SOW
    uint32_t gpstime = sow % 86400;

    // set model time in BDT
    m_sow = sow;
    m_grid.resize(320);

    for (std::size_t row = 10; row > 0; --row)
    {
        // we use a single row vector for m_grid
        // table 0 is IGP <= 160
        // table 1 is IGP > 160 -> has an offset of 160
        for (std::size_t table = 0; table <= 1; ++table)
        {
            for (std::size_t col = 0; col <= 15; ++col)
            {
                // calc IGP num formula: col * 10 + row
                // + table * 160, to access both IGP tables
                // array index is then -1
                std::size_t index = col * 10 + (table * 160) + row - 1;

                /// -1.0, because we are east of Greenwich
                double lambda = -1.0 * (col * 5.0 + 70);
                double phi = 1.0 * (row * 5.0 + 5.0) - table * 2.5;
                double corr = lcl_calcKlobucharCorrectionBDS(klob, gpstime, phi, lambda);
                m_grid[index] = IonoGridInfo(lcl_convertMeterToTECU(corr, bnav::BDS_B1I_FREQ));
            }
        }
    }
}


void Ionosphere::load(const SubframeBufferParam &sfbuf)
{
    // ensure correct type
    assert(sfbuf.type == SubframeBufferType::D2_ALMANAC);
    // ensure there is one subframe
    assert(sfbuf.data.size() == 1);

    // if there was already data loaded
    m_grid.clear();

    SubframeVector vfra5 = sfbuf.data[0];
    // ensure there are all pages
    assert(vfra5.size() == 120);

    // Pnum 1 to 13 of Frame 5
    processPageBlock(vfra5, 0);
    // ensure we have all IGPs
    assert(m_grid.size() == 160);

    // Pnum 61 to 73 of Frame 5
    processPageBlock(vfra5, 60);
    // ensure we have all IGPs
    assert(m_grid.size() == 320);

    // date of issue of ionospheric model is at page 1 of subframe 1
    // [1] 5.3.3.1 Basic NAV Information, p. 68
    // FIXME: this gets it from subframe 5...
    m_sow = vfra5.front().getSOW();
}

/**
 * @brief Ionosphere::processPageBlock Process one block of ionospheric pages.
 *
 * Both IGP tables are at separate page blocks: IGP<=160 is at pages 1 to 13
 * and IGP>160 is at 61 to 73.
 *
 * @param vfra5 SubframeVector of all 120 pages of frame 5.
 * @param startpage Index where the block starts.
 */
void Ionosphere::processPageBlock(const SubframeVector &vfra5, const std::size_t startpage)
{
    for (std::size_t i = startpage; i < startpage + 13; ++i)
    {
        uint32_t pnum = vfra5[i].getPageNum();
        assert(pnum == i + 1);

        NavBits<300> bits = vfra5[i].getBits();

        // page 13 ash 73 have reserved bits at the end of message
        parseIonospherePage(bits, pnum == 13 || pnum == 73);

        //std::cout << "pnum: " << pnum << std::endl;
    }
}

/**
 * @brief Ionosphere::parseIonospherePage Parse one page of ionospheric info.
 * @param bits message NavBits.
 * @param lastpage true, if this is page 13 or 73.
 */
void Ionosphere::parseIonospherePage(const NavBits<300> &bits, bool lastpage)
{
    // Ion1
    m_grid.push_back(IonoGridInfo(lcl_parsePageIon<50, 2, 60, 11>(bits)));
    // Ion2
    m_grid.push_back(IonoGridInfo(lcl_parsePageIon<71, 11, 90, 2>(bits)));
    // Ion3
    m_grid.push_back(IonoGridInfo(bits.getLeft<92, 13>()));
    // Ion4
    m_grid.push_back(IonoGridInfo(lcl_parsePageIon<105, 7, 120, 6>(bits)));

    // page 13 and 73 have no more data, bail out
    if (lastpage)
        return;

    // Ion5
    m_grid.push_back(IonoGridInfo(bits.getLeft<126, 13>()));
    // Ion6
    m_grid.push_back(IonoGridInfo(lcl_parsePageIon<139, 3, 150, 10>(bits)));
    // Ion7
    m_grid.push_back(IonoGridInfo(lcl_parsePageIon<160, 12, 180, 1>(bits)));
    // Ion8
    m_grid.push_back(IonoGridInfo(bits.getLeft<181, 13>()));
    // Ion9
    m_grid.push_back(IonoGridInfo(lcl_parsePageIon<194, 8, 210, 5>(bits)));
    // Ion10
    m_grid.push_back(IonoGridInfo(bits.getLeft<215, 13>()));
    // Ion11
    m_grid.push_back(IonoGridInfo(lcl_parsePageIon<228, 4, 240, 9>(bits)));
    // Ion12
    m_grid.push_back(IonoGridInfo(bits.getLeft<249, 13>()));
    // Ion13
    m_grid.push_back(IonoGridInfo(bits.getLeft<270, 13>()));

    //std::cout << "iono: " << iono << std::endl;
    //std::cout << "dt: " << m_grid.back().get_dt() << " givei: " << m_grid.back().get_give_index() << " give: " << m_grid.back().get_give() << std::endl;
}

bool Ionosphere::hasData() const
{
    return m_grid.size() == 320;
}

void Ionosphere::setSOW(const uint32_t sow)
{
    m_sow = sow;
}

/**
 * @brief Ionosphere::getSOW Get issue date of ionospheric model.
 * @return SOW.
 */
uint32_t Ionosphere::getSOW() const
{
    return m_sow;
}

/**
 * @brief getGrid Return the raw iono grid as single row vector.
 * @return vector of IonoGridInfo.
 */
std::vector<IonoGridInfo> Ionosphere::getGrid() const
{
    return m_grid;
}

void Ionosphere::setGrid(const std::vector<IonoGridInfo> &rhs)
{
    assert(rhs.size() == 320);
    m_grid.clear();
    for (auto it = rhs.cbegin(); it != rhs.cend(); ++it)
        m_grid.push_back(*it);
}

Ionosphere Ionosphere::diffToModel(const Ionosphere &rhs)
{
    Ionosphere diff;
    std::vector<IonoGridInfo> vdiff;
    std::vector<IonoGridInfo> gridrhs = rhs.getGrid();
    assert(m_grid.size() == gridrhs.size());

    // use SOW from current Ionosphere
    diff.setSOW(m_sow);

    for (std::size_t i = 0; i < m_grid.size(); ++i)
    {
        vdiff.push_back(m_grid[i] - gridrhs[i]);
    }
    diff.setGrid(vdiff);

    return diff;
}

/**
 * @brief Ionosphere::operator== Checks if two Ionospheric models are the same.
 * @param iono Ionosphere model.
 * @return true, if equal. false, if not.
 */
bool Ionosphere::operator==(const Ionosphere &iono) const
{
    assert(iono.getGrid().size() == m_grid.size());
    return (iono.getGrid() == m_grid);
}

/**
 * @brief Ionosphere::dump Dump complete IGP table.
 * @param rms Whether to print vertical delay or its RMS (GIVEI).
 */
void Ionosphere::dump(bool rms)
{
    std::cout << "DoI: " << m_sow << std::endl;
    for (std::size_t row = 10; row > 0; --row)
    {
        // we use a single row vector for m_grid
        // table 0 is IGP <= 160
        // table 1 is IGP > 160 -> has an offset of 160
        for (std::size_t table = 0; table <= 1; ++table)
        {
            for (std::size_t col = 0; col <= 15; ++col)
            {
                // calc IGP num formula: col * 10 + row
                // + table * 160, to access both IGP tables
                // array index is then -1
                std::size_t index = col * 10 + (table * 160) + row - 1;
                std::cout << std::setw(5)
                          << (rms ? m_grid[index].getGive_TECU() : m_grid[index].getVerticalDelay_TECU());
            }
            std::cout << std::endl;
        }
    }
}

/**
 * @brief Ionosphere::dump2 Dump two IGP tables (<=160 and >160).
 */
void Ionosphere::dump2(bool rms)
{
    std::cout << "DoI: " << m_sow << std::endl;
    std::cout << "IGP <= 160" << std::endl;
    // IGP <= 160
    for (std::size_t row = 10; row > 0; --row)
    {
        for (std::size_t col = 0; col <= 15; ++col)
        {
            // calc IGP num formula: col * 10 + row
            // array index is then -1
            std::size_t index = col * 10 + row - 1;
            std::cout << std::setw(5)
                      << (rms ? m_grid[index].getGive_TECU() : m_grid[index].getVerticalDelay_TECU());
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "IGP > 160" << std::endl;
    // IGP > 160
    for (std::size_t row = 170; row > 160; --row)
    {
        for (std::size_t col = 0; col <= 15; ++col)
        {
            // calc IGP num formula: col * 10 + row
            // array index is then -1
            std::size_t index = col * 10 + row - 1;
            std::cout << std::setw(5)
                      << (rms ? m_grid[index].getGive_TECU() : m_grid[index].getVerticalDelay_TECU());
        }
        std::cout << std::endl;
    }
}

} // namespace bnav
