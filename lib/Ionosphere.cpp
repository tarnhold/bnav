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

    // Pnum 1 to 13 and Pnum 61 to 73 of Frame 5 are Ionosphere in D2
    for (std::size_t i = 0; i < 13; ++i)
    {
        // FIXME: Seite 13 ist anders!
        uint32_t pnum = vfra5[i].getPageNum();
        assert(pnum == i + 1);

        NavBits<300> bits = vfra5[i].getBits();
        parseIonospherePage(bits);

        std::cout << pnum << std::endl;
    }

    // FIXME: Seite 73 ist auch wie 13, anders
}

void Ionosphere::parseIonospherePage(const NavBits<300> &bits)
{
    /*
    2, 11
    11, 2
    13
    7, 6
    */

    /*
    NavBits<13> iono = bits.getLeft<50, 2>();
    iono <<= 11;
    NavBits<11> lsb = bits.getLeft<60, 11>();
    iono ^= lsb;
*/

    NavBits<13> iono = lcl_parsePageIon<50, 2, 60, 11>(bits);
    //IonoGridInfo ion(iono);
    m_grid.push_back(IonoGridInfo(iono));

    std::cout << "iono: " << iono << std::endl;
    std::cout << "dt: " << m_grid.back().get_dt() << " givei: " << m_grid.back().get_give_index() << " give: " << m_grid.back().get_give() << std::endl;

}

} // namespace bnav
