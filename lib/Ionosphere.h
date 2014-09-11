#ifndef IONOSPHERE_H
#define IONOSPHERE_H

#include "NavBits.h"
#include "SubframeBuffer.h"
#include "DateTime.h"

#include "Ephemeris.h" // FIXME: maybe move KlobucharParam here

#include <vector>

namespace bnav
{

struct IonoGridDimension
{
    double latitude_north;
    double latitude_south;
    double latitude_spacing;
    double longitude_west;
    double longitude_east;
    double longitude_spacing;

    IonoGridDimension();
    IonoGridDimension(const double latnorth, const double latsouth, const double latspace,
                      const double longwest, const double longeast, const double longspace);

    std::size_t getItemCountLatitude() const;
    std::size_t getItemCountLongitude() const;
};

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
    DateTime m_datetime;
    std::vector<IonoGridInfo> m_grid;
    IonoGridDimension m_griddim;

public:
    Ionosphere();
    Ionosphere(const SubframeBufferParam &sfbuf, const uint32_t weeknum);
    Ionosphere(const KlobucharParam &klob, const DateTime &datetime, const bool global = false);

    void load(const SubframeBufferParam &sfbuf, const uint32_t weeknum);
    void load(const KlobucharParam &klob, const DateTime &datetime, const bool global = false);

    bool hasData() const;
    void setDateOfIssue(const DateTime &datetime);
    DateTime getDateOfIssue() const;

    void setGrid(const std::vector<IonoGridInfo> &rhs);
    std::vector<IonoGridInfo> getGrid() const;

    void setGridDimension(const IonoGridDimension &igd);
    IonoGridDimension getGridDimension() const;

    Ionosphere diffToModel(const Ionosphere &rhs);

    bool operator==(const Ionosphere &iono) const;

    void dump(bool rms = false);

private:
    void processPageBlock(const SubframeVector &vfra5, const std::size_t startpage, std::vector<IonoGridInfo> &grid_chinese);
    void parseIonospherePage(const NavBits<300> &bits, bool lastpage, std::vector<IonoGridInfo> &grid_chinese);
};

} // namespace bnav

#endif // IONOSPHERE_H
