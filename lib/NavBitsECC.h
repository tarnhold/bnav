#ifndef NAVBITSECC_H
#define NAVBITSECC_H

#include <cstdint>
#include <bitset>
#include <vector>

#include <iostream> // debug
#include <iomanip>

typedef std::bitset<15> subword;

namespace
{

/*
 * [1] Table 5-2 ROM table list for error correction
 */
static const std::vector< uint16_t > cROMTable = {
    0x0000, 0x0001, 0x0002, 0x0010, // 0 - 3
    0x0004, 0x0100, 0x0020, 0x0400, // 4 - 7
    0x0008, 0x4000, 0x0200, 0x0080, // 8 - 11
    0x0040, 0x2000, 0x0800, 0x1000  // 12 - 15
};


// @TODO: test this
// take 110110010010101
// and change a single bit for every position and check if it repairs correctly!

/*!
 * BCH(15,11,1) decoding
 *
 * [1] Chapter 5.1.3 Data Error Correction Code Mode
 *
 * @param 15 bits of a navigation message (11+4)
 * @return Index value of the ROM table
 *
 */
std::size_t decodeBCH(const subword &message)
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
    return d0 + (d1 << 1) + (d2 << 2) + (d3 << 3);
}

/*!
 * BCH(15,11,1) encoding
 *
 * [1] Chapter 5.1.3 Data Error Correction Code Mode
 *
 */
// @TODO: change this to take 15 bits, and ignore last 4 bits,
// just write parity bits into them
uint8_t encodeBCH(const std::bitset<11> &information)
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

}

namespace bnav
{

/*
 * More than one wrong bit can't be detected by BCH(15,11,1)!
 */
template <std::size_t len>
class NavBitsECC
{
private:
    std::bitset<len> m_bits;                          //< raw message bits
    std::vector< subword > m_msglist;     //< bit split into 15 bit parts
    std::size_t m_counter; //< how many subwords got fixed

public:
    NavBitsECC(const std::bitset<len> &bits);
    ~NavBitsECC();

    /*std::vector< subword >*/void splitMessage();
    std::string mergeMessage(/*std::vector< subword > msglist*/);

    bool checkAndFix(subword *pair);
    //bool checkAndFixParity(const std::bitset<len> &message);
    bool checkAndFixAll();
    
    bool isModified();

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
NavBitsECC<len>::NavBitsECC(const std::bitset<len> &bits)
    : m_bits(bits)
    , m_counter(0)
{
    // ensure we use only valid templates
    static_assert(len == 30 || len == 90 || len == 150 || len == 250,
        "invalid bitset size");
        
    checkAndFixAll();
}
    
template <std::size_t len>
NavBitsECC<len>::~NavBitsECC()
{
}


/*!
 * Splits the message into 11 bit information and 4 bit parity parts (here
 * called 'subword').
 *
 * E.g. 11+11+4+4 is handled as an array of (11+4, 11+4).
 *
 */
template <std::size_t len>
/*std::vector< subword >*/ void NavBitsECC<len>::splitMessage()
{
    //assert(messagestr.length() % 15 == 0);

    // @TODO: Sonderfall, len=15 (bitset<15>), dann braucht der quark unten nicht gemacht werden, einfach
    // direkt das eingabebitset in m_msglist packen.
    const std::size_t num = len / 15;

    //std::cout << "complete:" << messagestr.length() << std::endl;

    m_msglist.resize(num);
    for (std::size_t i = 0; i < num; ++i)
    {
        std::size_t startinfo = 11*i;
        std::size_t startpar = 11*num + 4*i;

        std::bitset<15> submessage;
//std::cerr << "startinfo:" << startinfo << " num: " << num << std::endl;
/* CASE mirrored message
 * frisst etwas zeit, das mirroring
        for (std::size_t k = 0; k < 12; ++k)
        {
            submessage[15 - k - 1] = message[startinfo + k];
        }

        for (std::size_t k = 0; k < 4; ++k)
        {
            submessage[15-11 - k - 1] = message[startpar + k];
        }
*/

        for (std::size_t k = 0; k < 12; ++k)
            submessage[15 - k - 1] = m_bits[len-1 - startinfo - k];

        for (std::size_t k = 0; k < 4; ++k)
            submessage[15-11 - k - 1] = m_bits[len-1 - startpar - k];

  //      std::cout << submessage << std::endl;

/* subword mit substr - fuer unittest nehmen
        subword submessage(
              messagestr.substr(11*i, 11)
            + messagestr.substr(11*num + 4*i, 4));
*/

        m_msglist[i] = submessage;

        //std::cout << i << ":" << submessage << std::endl;
    }

    //return m_msglist;
}

/*!
 * Concatenate an array (11+4,11+4,...) back to 11+11+...+4+4+...
 *
 * @TODO: don't use to_string()!
 */
template <std::size_t len>
std::string NavBitsECC<len>::mergeMessage(/*std::vector< subword > msglist*/)
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

    //std::cerr << "blubber: "
    //    << info << " : "<< parity << std::endl;

    return info + parity;
}

/*!
 * Check parity for one single subword (11+4)
 */
template <std::size_t len>
bool NavBitsECC<len>::checkAndFix(subword *message)
{
    std::size_t idx = decodeBCH(*message);

    // fix parity
    if (idx > 0)
    {
        subword fixed(*message xor std::bitset<15>(cROMTable[idx]));
        ++m_counter;
        
        std::cout << "parity check failed" << std::endl;
        std::cout << "old: " << *message << std::endl;
        std::cout << "new: " << fixed << std::endl;
        std::cout << std::setw(3) << idx << ": " << std::bitset<15>(cROMTable[idx]) << std::endl;
        
        return false;
    }
    
    return true;
}



template <std::size_t len>
bool NavBitsECC<len>::checkAndFixAll(/*const std::bitset<len> &message*/)
{
    splitMessage();
    
    //for (std::vector< subword >::iterator it = m_msglist.begin(); it != m_msglist.end(); ++it)
    //     checkAndFix(&(*it));
    for (std::size_t i = 0; i < m_msglist.size(); ++i)
        checkAndFix(&m_msglist[i]);
/*
    // 11+11+...+4+4+...
    std::vector< subword > vlist = splitMessage(message);
    
    // @TODO use returned value and save it into vlist[i]
    // at the moment we are using the uncorrected one
    for (std::vector< subword >::const_iterator it = vlist.begin(); it != vlist.end(); ++it)
         checkAndFixParity(*it);
*/    
   // std::cout << "orig :" << message << std::endl;
   // std::cout << "merge:" << mergeMessage(/*vlist*/) << std::endl;
 
   return true;
}

/*!
 * If the message was modified by fixing at least one subword this returns true.
 *
 * @return true if at least one subword was fixed. false if nothing was changed.
 */
template <std::size_t len>
bool NavBitsECC<len>::isModified()
{
    return m_counter > 0;
}


}

#endif // NAVBITSECC_H
