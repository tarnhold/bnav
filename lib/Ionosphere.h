#ifndef IONOSPHERE_H
#define IONOSPHERE_H

#include "NavBits.h"
#include "SubframeBuffer.h"

#include <vector>

namespace bnav
{

// [1] 5.3.3.8.2 Grid Ionospheric Vertical Error Index (GIVEI)
const double GIVEI_LOOKUP_TABLE[] = { 0.3, 0.6, 0.9, 1.2,
                                      1.5, 1.8, 2.1, 2.4,
                                      2.7, 3.0, 3.6, 4.5,
                                      6.0, 9.0, 15.0, 45.0};

class IonoGridInfo
{
     double m_dt;
     uint32_t m_givei;

public:
     IonoGridInfo();
     IonoGridInfo(const NavBits<13> &bits);

     double get_dt() const;
     uint32_t get_give_index() const;
     double get_give() const;
};

class Ionosphere
{
    std::vector<IonoGridInfo> m_grid;

public:
    Ionosphere();
    Ionosphere(const SubframeBufferParam &sfbuf);

    void dump();

private:
    void processPageBlock(const SubframeVector &vfra5, const std::size_t startpage);
    void parseIonospherePage(const NavBits<300> &bits, bool lastpage);
};

} // namespace bnav

#endif // IONOSPHERE_H
