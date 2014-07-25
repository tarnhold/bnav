#include "NavBitsECC.h"
#include "Subframe.h"

namespace bnav
{

Subframe::Subframe()
    : m_tow(0)
    , m_frameID(0)
    , m_pageNum(0)
    , m_isParityWordOneFixed(false)
    , m_isParityAllFixed(false)
{
}

NavBits<300> Subframe::getBits()
{
    return m_bits;
}

int Subframe::getTOW()
{
    return m_tow;
}

int Subframe::getFrameID()
{
    return m_frameID;
}

int Subframe::getPageNum()
{
    return m_pageNum;
}

bool Subframe::fixParityWordOne()
{
    NavBits<30> wordone = m_bits.getLeft<0, 30>();
    // HACK: process with raw bitset:
#include <bitset>
    std::bitset<30> bset = wordone.getBits();
    NavBitsECC<30> ecc(bset);
    ecc.checkAndFixAll();

    // TODO
    m_isParityWordOneFixed = false;
    return m_isParityWordOneFixed;
}

bool Subframe::fixParityAll()
{
    // TODO
    m_isParityAllFixed = false;
    return m_isParityAllFixed;
}

} // namespace bnav
