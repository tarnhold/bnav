#ifndef EPHEMERIS_H
#define EPHEMERIS_H

#include "SubframeBuffer.h"
#include "DateTime.h"

#include <stdint.h>

namespace bnav
{

struct KlobucharParam
{
    double alpha0;
    double alpha1;
    double alpha2;
    double alpha3;
    double beta0;
    double beta1;
    double beta2;
    double beta3;

    KlobucharParam()
        : alpha0(0.0)
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

#if 0
    // floating point comparisons...
    bool operator==(const KlobucharParam &rhs)
    {
        return (alpha0 == rhs.alpha0)
                && (alpha1 == rhs.alpha1)
                && (alpha2 == rhs.alpha2)
                && (alpha3 == rhs.alpha3)
                && (beta0 == rhs.beta0)
                && (beta1 == rhs.beta1)
                && (beta2 == rhs.beta2)
                && (beta3 == rhs.beta3);
    }
#endif
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

//    void dump();
//    void dump2();

private:
    void loadD1(const SubframeBufferParam &sfbuf);
    void loadD2(const SubframeBufferParam &sfbuf);

    void processD2Page1(const Subframe &sf);
    void processD2Page2(const Subframe &sf);
};

// non-members
#if 0
std::ostream & operator<<(std::ostream & out, const KlobucharParam & rhs)
{
    const std::size_t width = 10;
    out << std::setprecision(4);
    out << "alpha: "
        << std::setw(width) << rhs.alpha0 << " "
        << std::setw(width) << rhs.alpha1 << " "
        << std::setw(width) << rhs.alpha2 << " "
        << std::setw(width) << rhs.alpha3;
    out << std::endl;
    out << " beta: "
        << std::setw(width) << rhs.beta0 << " "
        << std::setw(width) << rhs.beta1 << " "
        << std::setw(width) << rhs.beta2 << " "
        << std::setw(width) << rhs.beta3;
    return out;
}
#endif

} // namespace bnav

#endif // EPHEMERIS_H
