#ifndef NAVBITSECC_H
#define NAVBITSECC_H

#include "NavBits.h"

#include <cstdint>
#include <string>
#include <vector>

#include <iostream>

typedef bnav::NavBits<15> subword;

namespace
{

/*
 * [1] Table 5-2 ROM table list for error correction
 */
const uint16_t cROMTable[] = {
    0x0000, 0x0001, 0x0002, 0x0010, // 0 - 3
    0x0004, 0x0100, 0x0020, 0x0400, // 4 - 7
    0x0008, 0x4000, 0x0200, 0x0080, // 8 - 11
    0x0040, 0x2000, 0x0800, 0x1000  // 12 - 15
};


/*!
 * BCH(15,11,1) decoding
 *
 * [1] Chapter 5.1.3 Data Error Correction Code Mode
 *
 * @param message 15 bits of a navigation message (11+4)
 * @return Index value of the ROM table
 *
 */
static inline std::size_t decodeBCH(const subword &message)
{
    // default state of shift registers is zero
    bool d0 = false, d1 = false, d2 = false, d3 = false;

    for (std::size_t i = message.size(); i > 0; --i)
    {
        // save old d3 state
        bool buf = d3;

        d3 = d2;
        d2 = d1;
        d1 = d0 xor buf;
        d0 = message[i-1] xor buf;
    }

    // put all together
    return static_cast<std::size_t>(d0 + (d1 << 1) + (d2 << 2) + (d3 << 3));
}

/*!
 * BCH(15,11,1) encoding
 *
 * [1] Chapter 5.1.3 Data Error Correction Code Mode
 *
 */
// @TODO: change this to take 15 bits, and ignore last 4 bits,
// just write parity bits into them
#if 0
uint8_t encodeBCH(const NavBits<11> &information)
{
    // default state of shift registers is zero
    bool d0 = false, d1 = false, d2 = false, d3 = false;

    for (std::size_t i = information.size(); i > 0; --i)
    {
        // save old d3 state
        bool buf = d3 xor information[i-1];

        d3 = d2;
        d2 = d1;
        d1 = d0 xor buf;
        d0 = buf;
    }

    //subword ret = information;
    return d0 + (d1 << 1) + (d2 << 2) + (d3 << 3);
}
#endif

}

namespace bnav
{

/*
 * More than one wrong bit can't be detected by BCH(15,11,1)!
 */
template <std::size_t len>
class NavBitsECCWord
{
private:
    NavBits<len> m_bits; ///< raw message bits
    std::vector< subword > m_msglist; ///< bit split into 15 bit parts
    std::size_t m_counter; ///< how many subwords got fixed

public:
    NavBitsECCWord(const NavBits<len> &bits);

    NavBits<len> getBits();

    bool isModified() const;
    std::size_t getModifiedCount() const;

private:
    void checkAndFixAllSubwords();
    void splitWordToSubword();
    std::string mergeSubwordsToWord();

    bool checkAndFixSubword(subword &message);

};

/* Implementation */

/*!
 * Transparently handles one word. Splits the message into its subword
 * components.
 *
 * Also adds functionality for 'special' words like
 *  11+11+..+i+4+4+..+i
 *  word6   i=6     bits=90
 *  word11  i=11    bits=150
 *  word18  i=18    bits=250
 *
 */
template <std::size_t len>
NavBitsECCWord<len>::NavBitsECCWord(const NavBits<len> &bits)
    : m_bits(bits)
    , m_counter(0)
{
    // ensure we use only valid templates
    static_assert(len == 15 || len == 30 || len == 90 || len == 150
                  || len == 270, "invalid NavBits size");

    splitWordToSubword();
    checkAndFixAllSubwords();
}

/*!
 * Splits the word into 11 bit information and 4 bit parity parts (here
 * called 'subword').
 *
 * normal word: 30 bits, 22 information, 8 parity
 * special words: ...
 *
 * E.g. 11+11+4+4 is handled as an array of (11+4, 11+4).
 *
 */
template <std::size_t len>
void NavBitsECCWord<len>::splitWordToSubword()
{
    static_assert(len % 15 == 0, "invalid size");

    const std::size_t num = len / 15;
    m_msglist.resize(num);

    // special case, if argument is NavBits<15>, we could just skip all the
    // splitting and save the bits directly into the list.
    if (num == 1)
    {
        m_msglist[0] = m_bits;
        return;
    }

    for (std::size_t i = 0; i < num; ++i)
    {
        std::size_t startinfo = 11*i;
        std::size_t startpar = 11*num + 4*i;

        NavBits<15> submessage;

        // note: this is accessed from right
        for (std::size_t k = 0; k < 12; ++k)
            submessage[15 - k - 1] = m_bits[len-1 - startinfo - k];

        for (std::size_t k = 0; k < 4; ++k)
            submessage[15-11 - k - 1] = m_bits[len-1 - startpar - k];

        m_msglist[i] = submessage;

        //std::cout << i << ":" << submessage << std::endl;
    }
}

/*!
 * Concatenate an array (11+4,11+4,...) back to 11+11+...+4+4+...
 *
 * TODO: don't use to_string()!
 */
template <std::size_t len>
std::string NavBitsECCWord<len>::mergeSubwordsToWord()
{
    // merge msg list back to original message
    // only if some parity bits were fixed!
    std::string info;
    std::string parity;

    for (std::vector< subword >::iterator it = m_msglist.begin(); it != m_msglist.end(); ++it)
    {
        std::string msgstr = it->to_string();
        //std::cerr << "blob: " <<msgstr << " : "<< msgstr.substr(0, 11) << " - " << msgstr.substr(11) << std::endl;
        info += msgstr.substr(0, 11);
        parity += msgstr.substr(11);
    }

    return info + parity;
}

/*!
 * Check parity for one single subword (11+4)
 */
template <std::size_t len>
bool NavBitsECCWord<len>::checkAndFixSubword(subword &message)
{
    std::size_t idx = decodeBCH(message);

    // fix parity
    if (idx > 0)
    {
        subword fixed(message xor NavBits<15>(cROMTable[idx]));
        ++m_counter;

//#if 0
        std::cout << "parity check failed" << std::endl;
        std::cout << "old: " << message << std::endl;
        std::cout << "new: " << fixed << std::endl;
        std::cout << std::setw(3) << idx << ": " << NavBits<15>(cROMTable[idx]) << std::endl;
//#endif

        message = fixed;

        return false;
    }

    return true;
}



template <std::size_t len>
void NavBitsECCWord<len>::checkAndFixAllSubwords()
{
    for (std::size_t i = 0; i < m_msglist.size(); ++i)
        checkAndFixSubword(m_msglist[i]);
}

template <std::size_t len>
NavBits<len> NavBitsECCWord<len>::getBits()
{
    std::string merge;
    merge = mergeSubwordsToWord();

    return NavBits<len>(merge);
}

/*!
 * If the message was modified by fixing at least one subword this returns true.
 *
 * @return true if at least one subword was fixed. false if nothing was changed.
 */
template <std::size_t len>
bool NavBitsECCWord<len>::isModified() const
{
    //std::cout << "m_counter: " << m_counter << std::endl;
    return m_counter > 0;
}

template <std::size_t len>
std::size_t NavBitsECCWord<len>::getModifiedCount() const
{
    return m_counter;
}


}

#endif // NAVBITSECC_H
