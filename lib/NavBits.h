#ifndef NAVBITS_H
#define NAVBITS_H

#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

namespace bnav
{

template<std::size_t dim> class NavBits
{
    std::bitset<dim> m_bitset;

public:
    NavBits();

    NavBits(const std::string & string);
    NavBits(const char* cstr);
    NavBits(const std::bitset<dim> & bitset);

    template <std::size_t len>
    NavBits(const std::bitset<len> & bitset);

    NavBits(const NavBits<dim> & bits);

    template <std::size_t len>
    NavBits(const NavBits<len> & bits);

    template <typename T>
    NavBits(const T val);

    bool operator[](std::size_t index) const;
    typename std::bitset<dim>::reference operator[](std::size_t index);

    bool atLeft(std::size_t index) const;

    NavBits<dim>& operator=(const NavBits<dim> &rhs);
    NavBits<dim> operator^(const NavBits<dim> &rhs) const;
    NavBits<dim>& operator^=(const NavBits<dim> &rhs);
    NavBits<dim>& operator<<=(std::size_t shift);
    bool operator==(const NavBits<dim> &rhs) const;

    void setLeft(std::size_t index, bool value = true);

    template<std::size_t len>
    void setLeft(std::size_t index, const NavBits<len> &bits);

    void flip(std::size_t index);
    void flipLeft(std::size_t index);
    void flipAll();

    std::size_t size() const;

    std::bitset<dim> getBits() const;

    template<std::size_t start, std::size_t len>
    NavBits<len> getLeft() const;

    void dumpDifferingBits(const NavBits<dim> &rhs) const;

    unsigned long to_ulong() const;
    double to_double(const int32_t scale_pow2 = 0) const;
    std::string to_string() const;
};

template<std::size_t dim>
NavBits<dim>::NavBits()
{
}

template<std::size_t dim>
NavBits<dim>::NavBits(const std::string & string)
{
    // warn if string won't fit dim, otherwise bits get lost
    assert(string.length() <= dim);
    m_bitset = std::bitset<dim>(string);
}

template<std::size_t dim>
NavBits<dim>::NavBits(const char* cstr)
{
    // warn if string won't fit dim, otherwise bits get lost
    assert(std::strlen(cstr) <= dim);
    m_bitset = std::bitset<dim>(cstr);
}

/**
 * @brief NavBits Constructor for std::bitset with same dim as NavBits.
 * @param bitset bitset with same dimension as NavBits.
 */
template<std::size_t dim>
NavBits<dim>::NavBits(const std::bitset<dim> & bitset)
    : m_bitset(bitset)
{
}

/**
 * @brief NavBits Constructor for std::bitset with smaller size than NavBits.
 * @param bitset bitset with a smaller dimension as NavBits.
 */
template<std::size_t dim>
template<std::size_t len>
NavBits<dim>::NavBits(const std::bitset<len> & bitset)
    : m_bitset(std::bitset<dim>(bitset.to_string()))
{
}

/**
 * @brief NavBits Constructor for NavBits with same size.
 * @param bits NavBits with the same size.
 */
template<std::size_t dim>
NavBits<dim>::NavBits(const NavBits<dim> & bits)
    : m_bitset(bits.getBits())
{
}

/**
 * Constructor for NavBits<len> parameter.
 *
 * Parameter bitset can have another len than the dimension of
 * this class bitset. Smaller sizes get a null padding at the MSB.
 */
template<std::size_t dim>
template<std::size_t len>
NavBits<dim>::NavBits(const NavBits<len> & bits)
{
    // warn if bits won't fit dim, otherwise bits get lost
    assert(len <= dim);
    m_bitset = std::bitset<dim>(bits.to_string());
}

template<std::size_t dim>
template<typename T>
NavBits<dim>::NavBits(const T val)
{
    // negative values work, but not the conversion back to long or string, so ignore them now
    assert(val >= 0);
    // warn if value won't fit dim, otherwise bits get lost
    assert(val < std::pow(2, dim));
    m_bitset = std::bitset<dim>(static_cast<unsigned long long>(val));
}

/**
 * @brief operator= Copy assignment operator.
 * @param rhs NavBits.
 * @return Pointer to this object.
 */
template<std::size_t dim>
NavBits<dim>& NavBits<dim>::operator=(const NavBits<dim> &rhs)
{
    m_bitset = rhs.getBits();
    return *this;
}

/** Access from right - LSB is [0] **/
template<std::size_t dim>
bool NavBits<dim>::operator[](std::size_t index) const
{
    return m_bitset[index];
}

/// access as reference
template<std::size_t dim>
typename std::bitset<dim>::reference NavBits<dim>::operator[](std::size_t index)
{
    return m_bitset[index];
}

/** Access from left - MSB is [0] **/
template<std::size_t dim>
bool NavBits<dim>::atLeft(std::size_t index) const
{
    return m_bitset[m_bitset.size() - 1 - index];
}

template<std::size_t dim>
NavBits<dim> NavBits<dim>::operator^(const NavBits<dim> &rhs) const
{
    return m_bitset ^ rhs.getBits();
}

template<std::size_t dim>
NavBits<dim>& NavBits<dim>::operator^=(const NavBits<dim> &rhs)
{
    m_bitset = m_bitset ^ rhs.getBits();
    return *this;
}

template<std::size_t dim>
NavBits<dim>& NavBits<dim>::operator<<=(std::size_t shift)
{
    m_bitset <<= shift;
    return *this;
}

/**
 * @brief operator == Check for equality of two NavBits<dim>.
 * @param rhs NavBits<dim> to compare to.
 * @return true if equal, false if unequal.
 */
template<std::size_t dim>
bool NavBits<dim>::operator==(const NavBits<dim> &rhs) const
{
    return m_bitset == rhs.getBits();
}

/**
 * @brief setLeft Set bit at position index starting from the left to value.
 * @param index index of the bit starting from the left.
 * @param value boolean to which the bit should be changed.
 */
template<std::size_t dim>
void NavBits<dim>::setLeft(std::size_t index, bool value)
{
    m_bitset.set(m_bitset.size() - 1 - index, value);
}

template<std::size_t dim>
template<std::size_t len>
void NavBits<dim>::setLeft(std::size_t index, const NavBits<len> &bits)
{
    assert(index + len <= dim);

    for (std::size_t i = index; i < index + len; ++i)
        setLeft(i, bits.atLeft(i - index));
}

template<std::size_t dim>
void NavBits<dim>::flip(std::size_t index)
{
    m_bitset.flip(index);
}

template<std::size_t dim>
void NavBits<dim>::flipLeft(std::size_t index)
{
    m_bitset.flip(m_bitset.size() - 1 - index);
}

template<std::size_t dim>
void NavBits<dim>::flipAll()
{
    for (std::size_t i = 0; i < m_bitset.size(); ++i)
        m_bitset.flip(i);
}

template<std::size_t dim>
std::size_t NavBits<dim>::size() const
{
    return m_bitset.size();
}

template<std::size_t dim>
std::bitset<dim> NavBits<dim>::getBits() const
{
    return m_bitset;
}

/**
 * Get a bitslice, index starting from left.
 *
 * getLeft<0,10>() will return the 10 most significant bits
 */
template<std::size_t dim>
template <std::size_t start, std::size_t len>
NavBits<len> NavBits<dim>::getLeft() const
{
    assert(start >= 0);
    assert(len >= 0);
    assert(start + len <= dim);

    NavBits<len> bTest;
    for (std::size_t i = start; i < start + len; ++i)
    {
        // save first bit into bTest[0]:
        bTest.setLeft(i - start, atLeft(i));
    }
    return bTest;
}

// debug stuff
/**
 * @brief differenceBits Debugging member to get the difference of two NavBits<dim>.
 * @param rhs NavBits<dim> to compare to.
 */
template<std::size_t dim>
void NavBits<dim>::dumpDifferingBits(const NavBits<dim> &rhs) const
{
    for (std::size_t i = 0; i < rhs.size(); ++i)
    {
        if (atLeft(i) != rhs.atLeft(i))
            std::cout << std::setw(3) << i << ": " << atLeft(i) << ":" << rhs.atLeft(i) << std::endl;
        // save first bit into bTest[0]:
        //bTest.setLeft(i - start, atLeft(i));
    }
    //return bTest;
}

/**
 * Conversion
 *
 */
// FIXME: be explicit: change to "uint32_t to_uint32_t() const"
template<std::size_t dim>
unsigned long NavBits<dim>::to_ulong() const
{
 return m_bitset.to_ulong();
}

/**
 * @brief to_double Convert value to double precision values, with sign ext.
 * @param scale_pow2 Scale by power of two.
 * @return Value converted to double with sign extension.
 */
template<std::size_t dim>
double NavBits<dim>::to_double(const int32_t scale_pow2) const
{
    double scale { std::pow(2, scale_pow2) };
    NavBits<1> sign = getLeft<0, 1>();
    NavBits<dim - 1> value = getLeft<1, dim - 1>();

    // negative value
    if (sign[0])
    {
        value.flipAll();

        return (-1.0 * static_cast<double>(value.to_ulong()) - 1.0) * scale;
    }

    return 1.0 * static_cast<double>(value.to_ulong()) * scale;
}

template<std::size_t dim>
std::string NavBits<dim>::to_string() const
{
 return m_bitset.to_string();
}

// non-members
template<std::size_t dim>
std::ostream & operator<<(std::ostream & out, const NavBits<dim> & rhs)
{
    out << rhs.getBits();
    return out;
}

} // namespace bnav

#endif // NAVBITS_H
