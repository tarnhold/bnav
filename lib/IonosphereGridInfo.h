#ifndef IONOSPHEREGRIDINFO_H
#define IONOSPHEREGRIDINFO_H

#include "NavBits.h"

#include <cstdint>

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
     void loadFromMeter(const double vertdelay, const double rms = 0.0);

     void setVerticalDelay_TECU(const uint32_t tec);

     uint32_t getVerticalDelay_TECU() const;
     uint32_t getGive_TECU() const;

     bool operator==(const IonoGridInfo &rhs) const;
     IonoGridInfo operator-(const IonoGridInfo &rhs) const;

private:
     void loadVerticalDelay(const NavBits<9> &bits);
     void loadGivei(const NavBits<4> &bits);
};

}

#endif // IONOSPHEREGRIDINFO_H
