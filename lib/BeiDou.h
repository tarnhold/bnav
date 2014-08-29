#ifndef BEIDOU_H
#define BEIDOU_H

#include <iostream>

namespace bnav
{

const double SPEED_OF_LIGHT = 2.99792458e8;

static const uint32_t SECONDS_OF_A_WEEK = 7*24*60*60;

static const uint32_t BDS_PREABMLE = 1810;

const double BDS_B1I_FREQ = 1561.098e6;
const double BDS_B2I_FREQ = 1207.140e6;

enum class SignalType
{
    BDS_B1,
    BDS_B2,
    NONE
};

#if 0
std::ostream & operator<<(std::ostream & out, const SignalType & rhs)
{
    switch (rhs)
    {
    case SignalType::BDS_B1:
        out << "B1";
        break;
    case SignalType::BDS_B2:
        out << "B2";
        break;
    case SignalType::NONE:
        out << "None";
        break;
    }

    return out;
}
#endif

} // namespace bnav

#endif // BEIDOU_H
