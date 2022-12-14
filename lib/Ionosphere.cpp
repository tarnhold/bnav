#include "Ionosphere.h"
#include "BeiDou.h"
#include "NavBits.h"
#include "SubframeBuffer.h"

#include <cmath>
#include <iostream>

namespace
{

/**
 * @brief lcl_calcKlobucharCorrectionBDS Calculate Klobuchar correction at position
 * (phi, lambda). This is only vertical delay, no slant!
 *
 * @param klob Klobuchar parameters.
 * @param time Local time.
 * @param phi Phi in degrees, west of Greenwich is negative.
 * @param lambda Lambda in degrees, south of equator is negative.
 * @return Vertical delay in meters.
 *
 * References:
 * [1] ICD, 5.2.4.7 Ionospheric Delay Model Parameters, pp. 25
 * [2] Ionospheric Time-Delay Algorithm for Single Frequency GPS Users,
 *     John. A. Klobuchar, 1987
 * [3] http://www.navipedia.net/index.php/Klobuchar_Ionospheric_Model
 */
double lcl_calcKlobucharCorrectionBDS(const bnav::KlobucharParam &klob, const uint32_t time, const double phi, const double lambda)
{
    assert(time < 86400);

    // convert to semicircle
    double semiphi { phi / 180.0 };
    double semilambda { lambda / 180.0 };

//    std::cout << "phi: " << semiphi << std::endl;

    // chinese don't do geomagnetic latitude
    double absphim { std::fabs(semiphi) };

//    std::cout << "phim: " << phim << std::endl;

    int32_t localtime { static_cast<int32_t>(std::lround(4.32e4 * semilambda)) + static_cast<int32_t>(time) };

    if (localtime > 86400)
        localtime = localtime - 86400;
    if (localtime < 0)
        localtime = localtime + 86400;

//    std::cout << "time: " << time2 << std::endl;

    double amplitude { klob.alpha0 + absphim * (klob.alpha1 + absphim * (klob.alpha2 + absphim * klob.alpha3)) };
    double period { klob.beta0 + absphim * (klob.beta1 + absphim * (klob.beta2 + absphim * klob.beta3)) };

    // amplitude is negative on the southern sphere
    // changing the sign isn't much better.
    if (amplitude < 0.0)
        amplitude = 0.0;
    if (period < 72000.0)
        period = 72000.0;
    // this causes the vertical line:
    if (period > 172800.0)
        period = 172800.0;

    double tiono;

    // ignore slant factor F, because we want vertical delay
    if (std::abs(localtime - 50400) < (period / 4.0))
    {
        // offset 50400 is 14h at local time, this is when ionospheric delay
        // reaches maximum usually
        const double x = { 2 * bnav::PI * (localtime - 50400) / period };

        //std::cout << "x: " << x << std::endl;

        tiono = 5.0e-9 + amplitude * std::cos(x);
    }
    else
        tiono = 5.0e-9;

    // we have time atm
    return tiono * bnav::SPEED_OF_LIGHT;
}

/**
 * @brief lcl_convertChineseToEuropeanGrid Convert chinese numbering to european
 * numbering. Chinese is column-wise bottom up from the left. European is
 * row-wise from top to bottom.
 *
 * Chinese (IGP <=160)     European
 *
 * 10  20 30 .. 160             1   2   3 ..  16
 * ..  .. .. ..  ..            17  18  19 ..  32
 *  3  13 23 ..  ..            ..  ..  .. ..  ..
 *  2  12 22 ..  ..           144  ..  .. .. 160
 *  1  11 21 .. 151
 *
 * @param grid_chinese Grid in chinese format, see ICD p. 72.
 */
std::vector<bnav::IonoGridInfo> lcl_convertChineseToEuropeanGrid(std::vector<bnav::IonoGridInfo> grid_chinese)
{
    std::vector<bnav::IonoGridInfo> grid;

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
                grid.push_back(grid_chinese[index]);
            }
        }
    }

    return grid;
}

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

    bnav::NavBits<13> iono { bits.getLeft<msb, msb_len>() };
    iono <<= lsb_len;
    // add lsb part to iono
    bnav::NavBits<lsb_len> lsbits { bits.getLeft<lsb, lsb_len>() };
    iono ^= lsbits;

    return iono;
}
}

namespace bnav
{

IonoGridDimension::IonoGridDimension()
    : latitude_north(0)
    , latitude_south(0)
    , latitude_spacing(0)
    , longitude_west(0)
    , longitude_east(0)
    , longitude_spacing(0)
{
}

IonoGridDimension::IonoGridDimension(const double latnorth, const double latsouth, const double latspace,
                  const double longwest, const double longeast, const double longspace)
    : latitude_north(latnorth)
    , latitude_south(latsouth)
    , latitude_spacing(latspace)
    , longitude_west(longwest)
    , longitude_east(longeast)
    , longitude_spacing(longspace)
{
    assert(latitude_north <= 90.0);
    assert(latitude_south >= -90.0);
    assert(latitude_north > latitude_south);
    assert(longitude_west >= -180.0);
    assert(longitude_east <= 180.0);
    assert(longitude_east > longitude_west);
    assert(latitude_spacing < 0.0);
    assert(longitude_spacing > 0.0);
}

std::uint32_t IonoGridDimension::getItemCountLatitude() const
{
    assert(latitude_spacing > 0.0 || latitude_spacing < 0.0);
#if 0
    std::cout << std::setprecision(1) << std::fixed
              << "lat max: " << latitude_north
              << " min: " << latitude_south
              << " space: " << latitude_spacing << std::endl;
#endif
    // add 0.5 because the compiler will always truncate
    // +1 because latmin and latmax are counting, too
    return static_cast<std::uint32_t>(std::fabs((latitude_north - latitude_south) / latitude_spacing) + 0.5) + 1;
}

std::uint32_t IonoGridDimension::getItemCountLongitude() const
{
    assert(longitude_spacing > 0.0 || longitude_spacing < 0.0);
#if 0
    std::cout << std::setprecision(1) << std::fixed
              << "long max: " << longitude_west
              << " min: " << longitude_east
              << " space: " << longitude_spacing << std::endl;
#endif
    return static_cast<std::uint32_t>(std::fabs((longitude_west - longitude_east) / longitude_spacing) + 0.5) + 1;
}

/**
 * @brief The Ionosphere class
 *
 * Stores IGP data into a single row vector.
 */
Ionosphere::Ionosphere()
    : m_datetime()
    , m_grid()
    , m_griddim()
{
}

Ionosphere::Ionosphere(const SubframeBufferParam &sfbuf, const uint32_t weeknum)
    : Ionosphere()
{
    load(sfbuf, weeknum);
}

Ionosphere::Ionosphere(const KlobucharParam &klob, const DateTime &datetime, const bool global)
    : Ionosphere()
{
    load(klob, datetime, global);
}

void Ionosphere::load(const KlobucharParam &klob, const DateTime &datetime, const bool global)
{
    // we need only time of day, not full SOW
    uint32_t secofday = datetime.getSOW() % 86400;

    // set model time in BDT
    m_datetime = datetime;

    if (global)
        m_griddim = IonoGridDimension(87.5, -87.5, -2.5, -180.0, 180.0, 5.0);
    else
        m_griddim = IonoGridDimension(55.0, 7.5, -2.5, 70.0, 145.0, 5.0);

    std::size_t rowcount = m_griddim.getItemCountLatitude();
    std::size_t colcount = m_griddim.getItemCountLongitude();
    std::cout << "rows: " << rowcount << " cols: " << colcount << std::endl;

    for (std::size_t row = 0; row < rowcount; ++row)
    {
        // we use a single row vector for m_grid
        for (std::size_t col = 0; col < colcount; ++col)
        {
            /// 1.0, because we are east of Greenwich
            double lambda { 1.0 * (m_griddim.longitude_west + static_cast<double>(col) * 5.0) };
            double phi { 1.0 * (m_griddim.latitude_north - static_cast<double>(row) * 2.5) };
            //std::cout << "phi: " << phi << std::endl;
            double corr { lcl_calcKlobucharCorrectionBDS(klob, secofday, phi, lambda) };
            IonoGridInfo info;
            info.loadFromMeter(corr);
            m_grid.push_back(info);
        }
    }

    if (global)
        assert(m_grid.size() == 5183);
        //std::cout << "size: " << m_grid.size() << std::endl;
    else
        assert(m_grid.size() == 320);
}

void Ionosphere::load(const SubframeBufferParam &sfbuf, const uint32_t weeknum)
{
    // ensure correct type
    assert(sfbuf.type == SubframeBufferType::D2_ALMANAC);
    // ensure there is one subframe
    assert(sfbuf.data.size() == 1);

    SubframeVector vfra5 = sfbuf.data[0];
    // ensure there are all pages
    assert(vfra5.size() == 120);

    // store grid data into single column vector with 320 elements
    std::vector<IonoGridInfo> grid_chinese;

    // Pnum 1 to 13 of Frame 5
    processPageBlock(vfra5, 0, grid_chinese);
    // ensure we have all IGPs
    assert(grid_chinese.size() == 160);

    // Pnum 61 to 73 of Frame 5
    processPageBlock(vfra5, 60, grid_chinese);
    // ensure we have all IGPs
    assert(grid_chinese.size() == 320);

    m_grid = lcl_convertChineseToEuropeanGrid(grid_chinese);
    m_griddim = IonoGridDimension(55.0, 7.5, -2.5, 70.0, 145.0, 5.0);

    // date of issue of ionospheric model is at page 1 of subframe 1
    // [1] 5.3.3.1 Basic NAV Information, p. 68
    // This gets it from subframe 5 page 1, which is ok, because for D2
    // subframes 1-5 have the same SOW.
    m_datetime.setTimeSystem(TimeSystem::BDT);
    m_datetime.setWeekAndSOW(weeknum, vfra5.front().getSOW());
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
void Ionosphere::processPageBlock(const SubframeVector &vfra5, const std::size_t startpage, std::vector<IonoGridInfo> &grid_chinese)
{
    for (std::size_t i = startpage; i < startpage + 13; ++i)
    {
        uint32_t pnum { vfra5[i].getPageNum() };
        assert(pnum == i + 1);

        NavBits<300> bits { vfra5[i].getBits() };

        // page 13 ash 73 have reserved bits at the end of message
        parseIonospherePage(bits, pnum == 13 || pnum == 73, grid_chinese);

        //std::cout << "pnum: " << pnum << std::endl;
    }
}

/**
 * @brief Ionosphere::parseIonospherePage Parse one page of ionospheric info.
 * @param bits message NavBits.
 * @param lastpage true, if this is page 13 or 73.
 */
void Ionosphere::parseIonospherePage(const NavBits<300> &bits, const bool lastpage, std::vector<IonoGridInfo> &grid_chinese)
{
    // Ion1
    grid_chinese.push_back(IonoGridInfo(lcl_parsePageIon<50, 2, 60, 11>(bits)));
    // Ion2
    grid_chinese.push_back(IonoGridInfo(lcl_parsePageIon<71, 11, 90, 2>(bits)));
    // Ion3
    grid_chinese.push_back(IonoGridInfo(bits.getLeft<92, 13>()));
    // Ion4
    grid_chinese.push_back(IonoGridInfo(lcl_parsePageIon<105, 7, 120, 6>(bits)));

    // page 13 and 73 have no more data, bail out
    if (lastpage)
        return;

    // Ion5
    grid_chinese.push_back(IonoGridInfo(bits.getLeft<126, 13>()));
    // Ion6
    grid_chinese.push_back(IonoGridInfo(lcl_parsePageIon<139, 3, 150, 10>(bits)));
    // Ion7
    grid_chinese.push_back(IonoGridInfo(lcl_parsePageIon<160, 12, 180, 1>(bits)));
    // Ion8
    grid_chinese.push_back(IonoGridInfo(bits.getLeft<181, 13>()));
    // Ion9
    grid_chinese.push_back(IonoGridInfo(lcl_parsePageIon<194, 8, 210, 5>(bits)));
    // Ion10
    grid_chinese.push_back(IonoGridInfo(bits.getLeft<215, 13>()));
    // Ion11
    grid_chinese.push_back(IonoGridInfo(lcl_parsePageIon<228, 4, 240, 9>(bits)));
    // Ion12
    grid_chinese.push_back(IonoGridInfo(bits.getLeft<249, 13>()));
    // Ion13
    grid_chinese.push_back(IonoGridInfo(bits.getLeft<270, 13>()));

    //std::cout << "iono: " << iono << std::endl;
    //std::cout << "dt: " << grid_chinese.back().get_dt()
    //          << " givei: " << grid_chinese.back().get_give_index()
    //          << " give: " << grid_chinese.back().get_give() << std::endl;
}

bool Ionosphere::hasData() const
{
    // regional grid has 320 cells, global 5183
    return m_grid.size() == 320 || m_grid.size() == 5183;
}

/**
 * @brief Ionosphere::setDateOfIssue Set issue date of ionospheric model.
 */
void Ionosphere::setDateOfIssue(const DateTime &datetime)
{
    m_datetime = datetime;
}

/**
 * @brief Ionosphere::getDateOfIssue Get issue date of ionospheric model.
 * @return SOW.
 */
DateTime Ionosphere::getDateOfIssue() const
{
    return m_datetime;
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
    // regional grid has 320 cells, global 5183
    assert(rhs.size() == 320 || rhs.size() == 5183);
    // FIXME: should set m_griddim, too - or at least fit to it
    m_grid.clear();
    for (auto it = rhs.cbegin(); it != rhs.cend(); ++it)
        m_grid.push_back(*it);
}

void Ionosphere::setGridDimension(const IonoGridDimension &igd)
{
    std::size_t newsize = igd.getItemCountLatitude() * igd.getItemCountLongitude();
    // we need to fit the grid to the new dimensions
    // ensure, we only expand, so we don't lose data
    assert(m_grid.size() <= newsize);
    m_grid.resize(newsize);
    m_griddim = igd;
}

IonoGridDimension Ionosphere::getGridDimension() const
{
    return m_griddim;
}

Ionosphere Ionosphere::diffToModel(const Ionosphere &rhs)
{
    Ionosphere diff;
    std::vector<IonoGridInfo> vdiff;
    const std::vector<IonoGridInfo> gridrhs = rhs.getGrid();
    assert(m_grid.size() == gridrhs.size());

    // use SOW from current Ionosphere
    diff.setDateOfIssue(m_datetime);

    for (std::size_t i = 0; i < m_grid.size(); ++i)
    {
        vdiff.push_back(m_grid[i] - gridrhs[i]);
    }
    diff.setGrid(vdiff);
    diff.setGridDimension(getGridDimension());

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
void Ionosphere::dump(const bool rms) const
{
    std::cout << "DoI: " << m_datetime.getDateTimeString() << " (" << m_datetime.getSOW() << ")" << std::endl;

    const std::size_t rowcount = m_griddim.getItemCountLatitude();
    const std::size_t colcount = m_griddim.getItemCountLongitude();
    std::cout << "rows: " << rowcount << " cols: " << colcount << std::endl;

    std::size_t index {0};
    for (std::size_t row = 0; row < rowcount; ++row)
    {
        // we use a single row vector for m_grid
        for (std::size_t col = 0; col < colcount; ++col)
        {
            std::cout << std::setw(5)
                      << (rms ? m_grid[index].getGive_TECU() : m_grid[index].getVerticalDelay_TECU());
            ++index;
        }
        std::cout << std::endl;
    }
}

} // namespace bnav
