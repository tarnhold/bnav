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

    KlobucharParam();

    KlobucharParam operator-(const KlobucharParam &rhs);

    bool operator==(const KlobucharParam &rhs) const;
    bool operator!=(const KlobucharParam &rhs) const;
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
