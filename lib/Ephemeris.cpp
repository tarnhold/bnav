#include "Ephemeris.h"

namespace bnav
{

Ephemeris::Ephemeris()
    : m_sow(0)
    , m_weeknum(0)
    , m_dateofissue()
    , m_klob(KlobucharParam())
{
}

Ephemeris::Ephemeris(const SubframeBufferParam &sfbuf)
    : Ephemeris()
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

uint32_t Ephemeris::getSOW() const
{
    return m_sow;
}

uint32_t Ephemeris::getWeekNum() const
{
    return m_weeknum;
}

DateTime Ephemeris::getDateOfIssue() const
{
    return m_dateofissue;
}

KlobucharParam Ephemeris::getKlobucharParam() const
{
    return m_klob;
}

bool Ephemeris::operator==(const Ephemeris &rhs) const
{
    return (getDateOfIssue() == rhs.getDateOfIssue())
            && (getKlobucharParam() == rhs.getKlobucharParam());
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

    //std::cout << "loadD2" << std::endl;

    processD2Page1(vfra[0]);
    processD2Page2(vfra[1]);
}

/**
 * @brief Ephemeris::processD2Page1 Parse page 1 of subframe 1 of D2
 * @param sf Subframe 1 page 1.
 *
 * [1] 5.3.2 D2 NAV Message Detailed structure, p. 44
 * [1] 5.2.4.4 Week Number (WN), p. 23
 */
void Ephemeris::processD2Page1(const Subframe &sf)
{
    assert(sf.getPageNum() == 1);
    NavBits<300> bits { sf.getBits() };

    // date of issue of ionospheric model is at page 1 of subframe 1
    // [1] 5.3.3.1 Basic NAV Information, p. 68
    // we read this already, use it
    m_sow = sf.getSOW();

    m_weeknum = bits.getLeft<64, 13>().to_ulong();
    m_dateofissue = DateTime(TimeSystem::BDT, m_weeknum, m_sow);

    // tgd1
    //std::cout << "tgd1 (ns): " << bits.getLeft<102, 10>().to_double(-1) << std::endl;

    // tgd2
    //std::cout << "tgd2: " << bits.getLeft<120, 10>() << std::endl;
}

/**
 * @brief Ephemeris::processD2Page2 Parse page 2 of subframe 1 of D2
 * @param sf Subframe 1 page 2.
 *
 * [1] 5.3.2 D2 NAV Message Detailed structure, p. 44
 * [1] 5.2.4.7 Ionospheric Delay Model Parameters, p. 25
 */
void Ephemeris::processD2Page2(const Subframe &sf)
{
    assert(sf.getPageNum() == 2);
    NavBits<300> bits { sf.getBits() };
    NavBits<64> allbits;

    // alpha0
    NavBits<8> kval = bits.getLeft<46, 6>();
    kval <<= 2;
    kval ^= bits.getLeft<60, 2>();
    allbits = kval;
    m_klob.alpha0 = kval.to_double(-30);
    // alpha1
    kval = bits.getLeft<62, 8>();
    allbits <<= 8;
    allbits ^= kval;
    m_klob.alpha1 = kval.to_double(-27);
    // alpha2
    kval = bits.getLeft<70, 8>();
    allbits <<= 8;
    allbits ^= kval;
    m_klob.alpha2 = kval.to_double(-24);
    // alpha3
    kval = bits.getLeft<78, 4>();
    kval <<= 4;
    kval ^= bits.getLeft<90, 4>();
    allbits <<= 8;
    allbits ^= kval;
    m_klob.alpha3 = kval.to_double(-24);

    // beta0
    kval = bits.getLeft<94, 8>();
    allbits <<= 8;
    allbits ^= kval;
    m_klob.beta0 = kval.to_double(11);
    // beta1
    kval = bits.getLeft<102, 8>();
    allbits <<= 8;
    allbits ^= kval;
    m_klob.beta1 = kval.to_double(14);
    // beta2
    kval = bits.getLeft<110, 2>();
    kval <<= 6;
    kval ^= bits.getLeft<120, 6>();
    allbits <<= 8;
    allbits ^= kval;
    m_klob.beta2 = kval.to_double(16);
    // beta3
    kval = bits.getLeft<126, 8>();
    allbits <<= 8;
    allbits ^= kval;
    m_klob.beta3 = kval.to_double(16);

    // save raw bits to avoid floating point comparisons
    m_klob.rawbits = allbits;
}

} // namespace bnav
