#ifndef IONOSPHERE_H
#define IONOSPHERE_H

#include "NavBits.h"
#include "SubframeBuffer.h"

#include <vector>

namespace bnav
{

class IonoGridInfo
{
     double m_dt;
     uint32_t m_dtraw;
     uint32_t m_givei;

public:
     IonoGridInfo();
     IonoGridInfo(const NavBits<13> &bits);

     double get_dt() const;
     uint32_t get_dtInTECU() const;
     uint32_t get_give_index() const;
     double get_give() const;

     bool operator==(const IonoGridInfo &rhs) const;
};

class Ionosphere
{
    uint32_t m_sow;
    std::vector<IonoGridInfo> m_grid;

public:
    Ionosphere();
    Ionosphere(const SubframeBufferParam &sfbuf);

// TODO    void load(const SubframeBufferParam &sfbuf);

    uint32_t getSOW() const;
    std::vector<IonoGridInfo> getGrid() const;

    bool operator==(const Ionosphere &iono) const;

    void dump();
    void dump2();

private:
    void processPageBlock(const SubframeVector &vfra5, const std::size_t startpage);
    void parseIonospherePage(const NavBits<300> &bits, bool lastpage);
};

} // namespace bnav

#endif // IONOSPHERE_H
