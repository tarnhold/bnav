#include "Ionosphere.h"
#include "NavBits.h"
#include "SubframeBuffer.h"

#include <iostream>

namespace
{
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
// TODO: initialize correctly (not with 0, because this is a valid value)
{
}

IonoGridInfo::IonoGridInfo(const NavBits<13> &bits)
{
    // [1] 5.3.2 D2 NAV Message Detailed structure, p. 50
    // first 9 bits are dt
    NavBits<9> dt = bits.getLeft<0, 9>();
    // last 4 bits are givei
    NavBits<4> givei = bits.getLeft<9, 4>();

    uint32_t ndt = dt.to_ulong();
    // maximum value is 63.875m, which is 511 when scaled by 0.125
    assert(ndt >= 0 && ndt <= 511);

    m_dt = ndt * 0.125;
    m_givei = givei.to_ulong();

    assert(m_givei >= 0 && m_givei <= 15);
}

double IonoGridInfo::get_dt() const
{
    return m_dt;
}

uint32_t IonoGridInfo::get_give_index() const
{
    return m_givei;
}

double IonoGridInfo::get_give() const
{
    return GIVEI_LOOKUP_TABLE[m_givei];
}

bool IonoGridInfo::operator==(const IonoGridInfo &rhs) const
{
    return rhs.get_dt() == m_dt
           && rhs.get_give_index() == m_givei;
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
    // TODO in load() schieben
    // unter die klasse etwas abstrahieren, damit eph/alm zusammenpassen

    // ensure correct type
    assert(sfbuf.type == SubframeBufferType::D2_ALMANAC);
    // ensure there is one subframe
    assert(sfbuf.data.size() == 1);

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
 */
void Ionosphere::dump()
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
                std::cout << std::setw(10) << m_grid[col * 10 + (table * 160) + row - 1].get_dt();
            }
            std::cout << std::endl;
        }
    }
}

/**
 * @brief Ionosphere::dump2 Dump two IGP tables (<=160 and >160).
 */
void Ionosphere::dump2()
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
            std::cout << std::setw(10) << m_grid[col * 10 + row - 1].get_dt();
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
            std::cout << std::setw(10) << m_grid[col * 10 + row - 1].get_dt();
        }
        std::cout << std::endl;
    }
}

} // namespace bnav
