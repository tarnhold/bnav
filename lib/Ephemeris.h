#ifndef EPHEMERIS_H
#define EPHEMERIS_H

#include "SubframeBuffer.h"
#include "DateTime.h"

#include <stdint.h>

namespace bnav
{

struct KlobucharParam
{
    NavBits<64> rawbits;
    double alpha0;
    double alpha1;
    double alpha2;
    double alpha3;
    double beta0;
    double beta1;
    double beta2;
    double beta3;

    KlobucharParam()
        : rawbits(0)
        , alpha0(0.0)
        , alpha1(0.0)
        , alpha2(0.0)
        , alpha3(0.0)
        , beta0(0.0)
        , beta1(0.0)
        , beta2(0.0)
        , beta3(0.0)
    {
    }

    KlobucharParam operator-(const KlobucharParam &rhs)
    {
        KlobucharParam ret;
        // rawbits are zero by default, leave it this way
        ret.alpha0 = std::fabs(alpha0 - rhs.alpha0);
        ret.alpha1 = std::fabs(alpha1 - rhs.alpha1);
        ret.alpha2 = std::fabs(alpha2 - rhs.alpha2);
        ret.alpha3 = std::fabs(alpha3 - rhs.alpha3);
        ret.beta0 = std::fabs(beta0 - rhs.beta0);
        ret.beta1 = std::fabs(beta1 - rhs.beta1);
        ret.beta2 = std::fabs(beta2 - rhs.beta2);
        ret.beta3 = std::fabs(beta3 - rhs.beta3);

        return ret;
    }

    bool operator==(const KlobucharParam &rhs) const
    {
        // avoid floating point comparisons by using the raw bits
        // if rawbits are zero, assume this is a differenced set (operator-)
        // split up into two 32 bits block, because ulong is only 32 bit
        return (rawbits.getLeft<0, 32>().to_ulong() != 0)
                && (rawbits.getLeft<0, 32>().to_ulong() != 0)
                && (rawbits == rhs.rawbits);
    }

    bool operator!=(const KlobucharParam &rhs) const
    {
        return !(*this == rhs);
    }
};

class Ephemeris
{
    uint32_t m_sow;
    uint32_t m_weeknum;
    DateTime m_dateofissue;
    KlobucharParam m_klob;

public:
    Ephemeris();
    Ephemeris(const SubframeBufferParam &sfbuf);

    void load(const SubframeBufferParam &sfbuf);

    uint32_t getSOW() const;
    uint32_t getWeekNum() const;
    DateTime getDateOfIssue() const;

    KlobucharParam getKlobucharParam() const;

    bool operator==(const Ephemeris &iono) const;

private:
    void loadD1(const SubframeBufferParam &sfbuf);
    void loadD2(const SubframeBufferParam &sfbuf);

    void processD1Subframe1(const Subframe &sf);
    void processD2Page1(const Subframe &sf);
    void processD2Page2(const Subframe &sf);
};

std::ostream & operator<<(std::ostream & out, const KlobucharParam & rhs);

} // namespace bnav

#endif // EPHEMERIS_H
