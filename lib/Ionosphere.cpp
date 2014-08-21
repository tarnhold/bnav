#include "Ionosphere.h"
#include "NavBits.h"
#include "SubframeBuffer.h"

#include <iostream>

namespace
{
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
{
}

IonoGridInfo::IonoGridInfo(const NavBits<13> &bits)
{
    // [1] 5.3.2 D2 NAV Message Detailed structure, p. 50
    // first 9 bits are dt
    NavBits<9> dt = bits.getLeft<0, 9>();
    // last 4 bits are givei
    NavBits<4> givei = bits.getLeft<9, 4>();

    m_dt = dt.to_ulong() * 0.125;
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

/**
 * @brief The Ionosphere class
 */
Ionosphere::Ionosphere()
{
}

Ionosphere::Ionosphere(const SubframeBufferParam &sfbuf)
{
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
}

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

void Ionosphere::dump()
{
    std::size_t igp_counter = 1;

    std::cout << "IGP <= 160" << std::endl;
    // IGP <= 160
    for (std::size_t row = 10; row > 0; --row)
    {
        for (std::size_t col = 0; col <= 15; ++col)
        {
            // calc IGP num formula: col * 10 + row
            // array index is then -1
            std::cout << std::setw(10) << m_grid[col * 10 + row].get_dt();

            ++igp_counter;
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
            std::cout << std::setw(10) << m_grid[col * 10 + row].get_dt();

            ++igp_counter;
        }

        std::cout << std::endl;
    }
}

} // namespace bnav
