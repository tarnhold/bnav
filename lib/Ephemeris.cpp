#include "Ephemeris.h"

namespace bnav
{

Ephemeris::Ephemeris()
    : m_sow(0)
{
}

Ephemeris::Ephemeris(const SubframeBufferParam &sfbuf)
    : m_sow(0)
{
    load(sfbuf);
}

void Ephemeris::load(const SubframeBufferParam &sfbuf)
{
    // ensure correct type
    assert(sfbuf.type == SubframeBufferType::D2_EPHEMERIS
           || sfbuf.type == SubframeBufferType::D1_EPHEMERIS);

    // two cases: D1 data / D2 data
    if (sfbuf.type == SubframeBufferType::D2_EPHEMERIS)
        loadD2(sfbuf);
    else
        loadD1(sfbuf);
}

void Ephemeris::loadD1(const SubframeBufferParam &sfbuf)
{
    (void)sfbuf;
    assert(false); // not implemented
}

void Ephemeris::loadD2(const SubframeBufferParam &sfbuf)
{
    // ensure there is one subframe
    assert(sfbuf.data.size() == 1);

    SubframeVector vfra = sfbuf.data[0];
    // ensure there are all pages
    assert(vfra.size() == 10);

    std::cout << "loadD2" << std::endl;

    processPage1(vfra[0]);

#if 0
    // Pnum 1 to 13 of Frame 5
    processPageBlock(vfra, 0);
    // ensure we have all IGPs
    assert(m_grid.size() == 160);
#endif

#if 0
// wrong, because it gets it from sf 5, see Ionosphere, too
    // date of issue of ionospheric model is at page 1 of subframe 1
    // [1] 5.3.3.1 Basic NAV Information, p. 68
    m_sow = vfra5.front().getSOW();
#endif
}

void Ephemeris::processPage1(const Subframe &sf)
{
    assert(sf.getPageNum() == 1);
    NavBits<300> bits = sf.getBits();

    //bits.getLeft<47, 5>()

    m_weeknum = bits.getLeft<64, 13>().to_ulong();

    // tgd1
    std::cout << "tgd1: " << bits.getLeft<102, 10>() << std::endl;

    // tgd2
    std::cout << "tgd2: " << bits.getLeft<120, 10>() << std::endl;

}

void Ephemeris::processPage2(const Subframe &sf)
{
    assert(sf.getPageNum() == 2);
    NavBits<300> bits = sf.getBits();

    // ..
    //std::cout << bits.getLeft<46, 6>() << std::endl;
    // alpha0
    bits.getLeft<46, 6>();
    bits.getLeft<60, 2>();

    // 1
    bits.getLeft<62, 8>();
    // 2
    bits.getLeft<70, 8>();
    // 3
    bits.getLeft<78, 4>();
    bits.getLeft<90, 4>();

    // beta0
    bits.getLeft<94, 8>();
    // 1
    bits.getLeft<102, 8>();
    // 2
    bits.getLeft<110, 2>();
    bits.getLeft<120, 6>();
    // 3
    bits.getLeft<126, 8>();
}

} // namespace bnav
