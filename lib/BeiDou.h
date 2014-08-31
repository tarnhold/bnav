#ifndef BEIDOU_H
#define BEIDOU_H

#include <iostream>
#include <stdint.h>

namespace bnav
{

constexpr double PI = 3.14159265358979323846;
constexpr double SPEED_OF_LIGHT = 2.99792458e8;

constexpr uint32_t SECONDS_OF_A_WEEK = 7*24*60*60;

constexpr uint32_t BDS_PREABMLE = 1810;

constexpr double BDS_B1I_FREQ = 1561.098e6;
constexpr double BDS_B2I_FREQ = 1207.140e6;

constexpr uint32_t BDS_MAX_PRN = 37;
constexpr uint32_t BDS_MAX_PRN_GEO = 5;

constexpr std::size_t D1_FRAME_SIZE[] = {1, 1, 1, 24, 24};
// one subframe has a duration of 6s
constexpr uint32_t D1_SUBFRAME_DURATION = 6;

constexpr std::size_t D2_FRAME_SIZE[] = {10, 6, 6, 6, 120};
// one frame has a duration of 3s (0.6s * 5)
// D2 has the same SOW for all subframes of one frame
constexpr uint32_t D2_FRAME_DURATION = 3;

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
