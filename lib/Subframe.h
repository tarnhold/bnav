#ifndef SUBFRAME_H
#define SUBFRAME_H

#include "NavBits.h"

namespace bnav
{

class Subframe
{
    NavBits<300> m_bits;
    int m_tow;
    int m_frameID;
    int m_pageNum;

    bool m_isParityWordOneFixed;
    bool m_isParityAllFixed;

public:
    Subframe();

    NavBits<300> getBits();
    int getTOW();
    int getFrameID();
    int getPageNum();

    bool fixParityWordOne();
    bool fixParityAll();
};

} // namespace bnav

#endif // SUBFRAME_H
