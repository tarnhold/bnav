#ifndef IONOSPHERE_H
#define IONOSPHERE_H

#include "NavBits.h"
#include "SubframeBuffer.h"

#include <vector>

namespace bnav
{

class IonoGridInfo
{
     uint32_t m_dtraw; ///< Raw dt value
     double m_dt; ///< dt converted to meters
     uint32_t m_dtTECU; ///< dt converted to 0.1 TECU
     uint32_t m_givei; ///< GIVE index value
     uint32_t m_giveTECU; ///< GIVE converted to 0.1 TECU

public:
     IonoGridInfo();
     IonoGridInfo(const NavBits<13> &bits);

     void load(const NavBits<13> &bits);

     uint32_t getVerticalDelay_TECU() const;
     uint32_t getGive_TECU() const;
#if 0
     uint32_t getGiveIndex() const;
     double getVerticalDelay_Meter() const;
     double getGive_Meter() const;
#endif

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

    void dump(bool rms = false);
    void dump2(bool rms = false);

private:
    void processPageBlock(const SubframeVector &vfra5, const std::size_t startpage);
    void parseIonospherePage(const NavBits<300> &bits, bool lastpage);
};

} // namespace bnav

#endif // IONOSPHERE_H
