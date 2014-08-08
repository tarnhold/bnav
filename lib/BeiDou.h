#ifndef BEIDOU_H
#define BEIDOU_H

#include <iostream>

namespace bnav
{

static const uint32_t BDS_PREABMLE = 1810;

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
