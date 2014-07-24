#ifndef NAVBITS_H
#define NAVBITS_H

#include <bitset>
#include <cassert>
#include <iostream> // debug
#include <string>

namespace bnav
{

//typedef NavBits<11> NavSubWordInformation;

template<int dim> class NavBits
{
protected:
	std::bitset<dim> m_bitset;
	std::size_t m_bitpos;

public:
	NavBits()
		: m_bitpos(0)
	{
	}

	NavBits(const int val)
		: m_bitpos(0)
	{
		m_bitset = std::bitset<dim>(val);
	}

	NavBits(const std::string & string)
		: m_bitpos(0)
	{
		m_bitset = std::bitset<dim>(string);
	}
	~NavBits() {};

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

	/// access as reference
	typename std::bitset<dim>::reference atLeft(std::size_t index)
	{
		return m_bitset[m_bitset.size() - 1 - index];
	}

    typename std::bitset<dim>& operator<<=(std::size_t shift)
    {
        m_bitset <<= shift;
        return m_bitset;
    }

	void set(std::size_t index)
	{
		m_bitset.set(m_bitset.size() - 1 - index);
	}

	void set(std::size_t index, bool value)
	{
		m_bitset.set(m_bitset.size() - 1 - index, value);
	}

	std::size_t size()
	{
		return m_bitset.size();
	}

	std::bitset<dim> get() const
	{
		return m_bitset;
	}

    /**
     * Get a bitslice, index starting from left.
     *
     * getLeft<0,10>() will return the 10 most significant bits
     */
    template<int start, int end>
    NavBits<end - start> getLeft() const
    {
        assert(end - start > 0);
        assert(end - start <= dim);

        NavBits<end - start> bTest;
        for (std::size_t i = start; i < end; ++i)
        {
            // save first bit into bTest[0]:
            bTest.atLeft(i - start) = atLeft(i);
        }
        return bTest;
    }

	/// optional: get as reference
	typename std::bitset<dim>::reference get()
	{
		return m_bitset;
	}

#if 0
    uint8_t as_uint8_t(uint8_t start, uint8_t end)
    {

    }
#endif

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


    /**
     * Conversion
     *
     */
     unsigned long to_ulong() const
     {
         return m_bitset.to_ulong();
     }
};


typedef NavBits<30> NavWord;
typedef NavBits<15> NavSubWord;


// non-member functions

// hat nichts mit der klasse an sich zu tun!
template<int dim>
std::ostream & operator<<(std::ostream & out, const NavBits<dim> & rhs)
	{
		out << rhs.get();
		return out;
	}

}




#endif // NAVBITS_H
