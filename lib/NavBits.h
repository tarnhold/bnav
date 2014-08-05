#ifndef NAVBITS_H
#define NAVBITS_H

#include <bitset>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

namespace bnav
{

template<std::size_t dim> class NavBits
{
protected:
    std::bitset<dim> m_bitset;
    std::size_t m_bitpos;

public:
    NavBits()
        : m_bitpos(0)
    {
    }

    NavBits(const std::string & string)
        : m_bitpos(0)
    {
        // warn if string won't fit dim, otherwise bits get lost
        assert(string.length() <= dim);
        m_bitset = std::bitset<dim>(string);
    }

    NavBits(const char* cstr)
        : m_bitpos(0)
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
        : m_bitpos(0)
    {
        m_bitset = bitset;
    }

    /**
     * @brief NavBits Constructor for std::bitset with smaller size than NavBits.
     * @param bitset bitset with a smaller dimension as NavBits.
     */
    template <std::size_t len>
    NavBits(const std::bitset<len> & bitset)
        : m_bitpos(0)
    {
        m_bitset = std::bitset<dim>(bitset.to_string());
    }

    /**
     * @brief NavBits Constructor for NavBits with same size.
     * @param bits NavBits with the same size.
     */
    NavBits(const NavBits<dim> & bits)
        : m_bitpos(0)
    {
        m_bitset = bits.getBits();
    }

    /**
     * Constructor for NavBits<len> parameter.
     *
     * Parameter bitset can have another len than the dimension of
     * this class bitset. Smaller sizes get a null padding at the MSB.
     */
    template <std::size_t len>
    NavBits(const NavBits<len> & bits)
        : m_bitpos(0)
    {
        // warn if bits won't fit dim, otherwise bits get lost
        assert(len <= dim);
        m_bitset = std::bitset<dim>(bits.to_string());
    }

    template <typename T>
    NavBits(const T val)
        : m_bitpos(0)
    {
        // negative values work, but not the conversion back to long or string, so ignore them now
        assert(val >= 0);
        // warn if value won't fit dim, otherwise bits get lost
        assert(val < std::pow(2, dim));
        m_bitset = std::bitset<dim>(val);
    }

    ~NavBits()
    {
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

#if 0
    template<int len>
    void setLeft(std::size_t index, const NavBits<len> &bits)
    {
        assert(index + len <= dim);

        for (std::size_t i = index; i < index + len; ++i)
        {
            // save first bit into [0]:
            setLeft(i - index, bits.atLeft(i));
        }
    }
#endif

    void flip(std::size_t index)
    {
        m_bitset.flip(index);
    }

    void flipLeft(std::size_t index)
    {
        m_bitset.flip(m_bitset.size() - 1 - index);
    }

    std::size_t size() const
    {
        return m_bitset.size();
    }

    std::bitset<dim> getBits() const
    {
        return m_bitset;
    }

#if 0
    /// optional: get as reference
    typename std::bitset<dim>::reference get()
    {
        return m_bitset;
    }
#endif

    /**
     * Get a bitslice, index starting from left.
     *
     * getLeft<0,10>() will return the 10 most significant bits
     */
    template<int start, int len>
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

#if 0
    uint8_t as_uint8_t(uint8_t start, uint8_t end)
    {

    }
#endif

#if 0
    void begin()
    {
        m_bitpos = 0;
    }

    // getNext<11>()
    // getNext<15>()
    template <int length>
    NavBits<length> getNext()
    {
        NavBits<length> nextbits;
        std::size_t start = m_bitpos;
        std::size_t end = m_bitpos + length;

std::cout << "m_bitpos:" << m_bitpos << " start:" << start << " end:" << end << std::endl;
        // ensure it is within bitset bounds
        //assert(end <= m_bitset.size());

        // get bits from m_bitpos till m_bitpos + length
        while (m_bitpos < end)
        {
            //FIXME: +1 and -1, this seems crazy
            std::cout << "nextbits:" << m_bitpos - start
                << " m_bitset:" << m_bitset.size() - m_bitpos
                << " value:" << m_bitset[m_bitset.size() -1 - m_bitpos]
                << std::endl;
            nextbits[m_bitpos - start + 1] = m_bitset[m_bitset.size() - 1 - m_bitpos];
            ++m_bitpos;
        }

        return nextbits;
    }

    void end()
    {
        m_bitpos = m_bitset.length();
    }
#endif

    /**
     * Conversion
     *
     */
     unsigned long to_ulong() const
     {
         return m_bitset.to_ulong();
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
