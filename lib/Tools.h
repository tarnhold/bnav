#ifndef TOOLS_H
#define TOOLS_H

#include <cassert>
#include <cstdint>
#include <string>
#include <limits>

namespace bnav
{

inline uint32_t stoui32(const std::string& s, std::size_t* idx = nullptr, const int base = 10)
{
    unsigned long const val{ std::stoul(s, idx, base) };
    assert(val <= std::numeric_limits<uint32_t>::max());
    return std::uint32_t(val);
}

template<typename T> inline typename std::enable_if<std::is_unsigned<T>::value, bool>::type checked_sub(T a, T b, T& result)
{
    if (a < b) {
        return true;
    }

    result = a - b;
    return false;
}

} // namespace bnav

#endif // TOOLS_H
