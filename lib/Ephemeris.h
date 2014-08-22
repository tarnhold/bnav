#ifndef EPHEMERIS_H
#define EPHEMERIS_H

#include "SubframeBuffer.h"

#include <stdint.h>

namespace bnav
{

class Ephemeris
{
    uint32_t m_sow;
    uint32_t m_weeknum;
    double m_tgd1;
    double m_tgd2;
//    std::vector<IonoGridInfo> m_grid;

public:
    Ephemeris();
    Ephemeris(const SubframeBufferParam &sfbuf);

    void load(const SubframeBufferParam &sfbuf);

    uint32_t getSOW() const;
    //std::vector<IonoGridInfo> getGrid() const;

//    bool operator==(const Ionosphere &iono) const;

//    void dump();
//    void dump2();

private:
    void loadD1(const SubframeBufferParam &sfbuf);
    void loadD2(const SubframeBufferParam &sfbuf);

    void processPage1(const Subframe &sf);
    void processPage2(const Subframe &sf);
//    void processPageBlock(const SubframeVector &vfra5, const std::size_t startpage);
//    void parseIonospherePage(const NavBits<300> &bits, bool lastpage);
};

} // namespace bnav

#endif // EPHEMERIS_H
