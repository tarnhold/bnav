#include "SubframeBuffer.h"

namespace bnav
{

/**
 * @brief SubframeBuffer::SubframeBuffer
 *
 * Currently we only handle complete data sets. That means, pages have to be
 * complete and without gaps (SOW and Pnum cheched). Incomplete data sets
 * get discarded at the moment.
 *
 * Data sets means Ephemeris, Almanac and Integrity data.
 */

SubframeBuffer::SubframeBuffer()
    : m_lastsow(0)
{
    // reserve space for five subframes
    m_buffer = SubframeVector(5);
}

/**
 * @brief SubframeBuffer::hasIncompleteData Checks if there is any dataset left.
 *
 * Used to check if there is incomplete data inside the buffer.
 *
 * @return true if there is data, false if empty.
 */
bool SubframeBuffer::hasIncompleteData()
{
    return m_buffer[0].size() > 0
            || m_buffer[1].size() > 0
            || m_buffer[2].size() > 0
            || m_buffer[3].size() > 0
            || m_buffer[4].size() > 0;
}

/**
 * @brief SubframeBuffer::checkLastSOW Check if the current SOW fits to the last
 * one in the buffer.
 * @param currentsow Current SOW.
 * @param duration Duration of a Subframe.
 */
void SubframeBuffer::checkLastSOW(uint32_t currentsow, uint32_t duration)
{
    // Last message SOW should fit the current one + 6s. So we can
    // easily detect data gaps.
    if ((m_lastsow + duration != currentsow) && (m_lastsow > 0))
    {
        std::cout << "Warning: SubframeBuffer: Data gap of "
                  << currentsow - (m_lastsow + duration)
                  << "s detected!" << std::endl;

        // initialize new data sets, because the old are uncompleted
        clearEphemerisData();
        clearAlmanacData();
    }
}

} // namespace bnav
