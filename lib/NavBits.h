#ifndef NAVBITS_H
#define NAVBITS_H

#include <bitset>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdint.h>
#include <string>

namespace bnav
{

template<std::size_t dim> class NavBits
{
    std::bitset<dim> m_bitset;

public:
    NavBits()
    {
    }

    NavBits(const std::string & string)
    {
        // warn if string won't fit dim, otherwise bits get lost
        assert(string.length() <= dim);
        m_bitset = std::bitset<dim>(string);
    }

    NavBits(const char* cstr)
    {
        // warn if string won't fit dim, otherwise bits get lost
        assert(std::strlen(cstr) <= dim);
        m_bitset = std::bitset<dim>(cstr);
    }

    /**
     * @brief NavBits Constructor for std::bitset with same dim as NavBits.
     * @param bitset bitset with same dimension as NavBits.
     */
    NavBits(const std::bitset<dim> & bitset)
        : m_bitset(bitset)
    {
    }

    /**
     * @brief NavBits Constructor for std::bitset with smaller size than NavBits.
     * @param bitset bitset with a smaller dimension as NavBits.
     */
    template <std::size_t len>
    NavBits(const std::bitset<len> & bitset)
        : m_bitset(std::bitset<dim>(bitset.to_string()))
    {
    }

    /**
     * @brief NavBits Constructor for NavBits with same size.
     * @param bits NavBits with the same size.
     */
    NavBits(const NavBits<dim> & bits)
        : m_bitset(bits.getBits())
    {
    }

    /**
     * Constructor for NavBits<len> parameter.
     *
     * Parameter bitset can have another len than the dimension of
     * this class bitset. Smaller sizes get a null padding at the MSB.
     */
    template <std::size_t len>
    NavBits(const NavBits<len> & bits)
    {
        // warn if bits won't fit dim, otherwise bits get lost
        assert(len <= dim);
        m_bitset = std::bitset<dim>(bits.to_string());
    }

    template <typename T>
    NavBits(const T val)
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
    NavBits<dim>& operator=(const NavBits<dim> &rhs)
    {
        m_bitset = rhs.getBits();
        return *this;
    }

    /** Access from right - LSB is [0] **/
    bool operator[](std::size_t index) const
    {
        return m_bitset[index];
    }

    /// access as reference
    typename std::bitset<dim>::reference operator[](std::size_t index)
    {
        return m_bitset[index];
    }

    /** Access from left - MSB is [0] **/
    bool atLeft(std::size_t index) const
    {
        return m_bitset[m_bitset.size() - 1 - index];
    }

    NavBits<dim> operator^(const NavBits<dim> &rhs) const
    {
        return m_bitset ^ rhs.getBits();
    }

    NavBits<dim>& operator^=(const NavBits<dim> &rhs)
    {
        m_bitset = m_bitset ^ rhs.getBits();
        return *this;
    }

    NavBits<dim>& operator<<=(std::size_t shift)
    {
        m_bitset <<= shift;
        return *this;
    }

    /**
     * @brief operator == Check for equality of two NavBits<dim>.
     * @param rhs NavBits<dim> to compare to.
     * @return true if equal, false if unequal.
     */
    bool operator==(const NavBits<dim> &rhs)
    {
        return m_bitset == rhs.getBits();
    }

    /**
     * @brief setLeft Set bit at position index starting from the left to true.
     * @param index index of the bit starting from the left.
     */
    void setLeft(std::size_t index)
    {
        m_bitset.set(m_bitset.size() - 1 - index);
    }

    /**
     * @brief setLeft Set bit at position index starting from the left to value.
     * @param index index of the bit starting from the left.
     * @param value boolean to which the bit should be changed.
     */
    void setLeft(std::size_t index, bool value)
    {
        m_bitset.set(m_bitset.size() - 1 - index, value);
    }

    template<std::size_t len>
    void setLeft(std::size_t index, const NavBits<len> &bits)
    {
        assert(index + len <= dim);

        for (std::size_t i = index; i < index + len; ++i)
            setLeft(i, bits.atLeft(i - index));
    }

    void flip(std::size_t index)
    {
        m_bitset.flip(index);
    }

    void flipLeft(std::size_t index)
    {
        m_bitset.flip(m_bitset.size() - 1 - index);
    }

    void flipAll()
    {
        for (std::size_t i = 0; i < m_bitset.size(); ++i)
            m_bitset.flip(i);
    }

    std::size_t size() const
    {
        return m_bitset.size();
    }

    std::bitset<dim> getBits() const
    {
        return m_bitset;
    }

    /**
     * Get a bitslice, index starting from left.
     *
     * getLeft<0,10>() will return the 10 most significant bits
     */
    template<std::size_t start, std::size_t len>
    NavBits<len> getLeft() const
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
    void dumpDifferingBits(const NavBits<dim> &rhs)
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
    unsigned long to_ulong() const
    {
     return m_bitset.to_ulong();
    }

    /**
     * @brief to_double Convert value to double precision values, with sign ext.
     * @param scale_pow2 Scale by power of two.
     * @return Value converted to double with sign extension.
     */
    double to_double(const int32_t scale_pow2 = 0) const
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

    std::string to_string() const
    {
     return m_bitset.to_string();
    }
};

// non-members
template<std::size_t dim>
std::ostream & operator<<(std::ostream & out, const NavBits<dim> & rhs)
{
    out << rhs.getBits();
    return out;
}

} // namespace bnav

#endif // NAVBITS_H
