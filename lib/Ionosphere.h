#ifndef IONOSPHERE_H
#define IONOSPHERE_H

#include "NavBits.h"
#include "SubframeBuffer.h"

#include "Ephemeris.h" // FIXME: maybe move KlobucharParam here

#include <vector>

namespace bnav
{

class IonoGridInfo
{
     uint32_t m_dtTECU; ///< dt converted to 0.1 TECU
     uint32_t m_giveTECU; ///< GIVE converted to 0.1 TECU
     bool m_isValid;

public:
     IonoGridInfo();
     IonoGridInfo(const uint32_t vertdelay, const uint32_t rms = 0);
     IonoGridInfo(const NavBits<13> &bits);

     void load(const uint32_t vertdelay, const uint32_t rms = 0);
     void load(const NavBits<13> &bits);

     uint32_t getVerticalDelay_TECU() const;
     uint32_t getGive_TECU() const;

     bool operator==(const IonoGridInfo &rhs) const;
     IonoGridInfo operator-(const IonoGridInfo &rhs) const;

private:
     void loadVerticalDelay(const NavBits<9> &bits);
     void loadGivei(const NavBits<4> &bits);
};

class Ionosphere
{
    uint32_t m_sow;
    std::vector<IonoGridInfo> m_grid;

public:
    Ionosphere();
    Ionosphere(const SubframeBufferParam &sfbuf);
    Ionosphere(const KlobucharParam &klob, const uint32_t time);

    void load(const SubframeBufferParam &sfbuf);
    void load(const KlobucharParam &klob, const uint32_t sow);

    bool hasData() const;
    void setSOW(const uint32_t sow);
    uint32_t getSOW() const;

    void setGrid(const std::vector<IonoGridInfo> &rhs);
    std::vector<IonoGridInfo> getGrid() const;

    Ionosphere diffToModel(const Ionosphere &rhs);

    bool operator==(const Ionosphere &iono) const;

    void dump(bool rms = false);
    void dump2(bool rms = false);

private:
    void processPageBlock(const SubframeVector &vfra5, const std::size_t startpage);
    void parseIonospherePage(const NavBits<300> &bits, bool lastpage);
};

} // namespace bnav

#endif // IONOSPHERE_H
