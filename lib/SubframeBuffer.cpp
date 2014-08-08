#include "SubframeBuffer.h"

// TODO: this is D1 only at the moment

namespace bnav
{

static const std::vector< std::size_t > D1_FRAME_SIZE = {1, 1, 1, 24, 24};
// one subframe is 6s long
static const std::size_t D1_SUBFRAME_DURATION = 6;

SubframeBuffer::SubframeBuffer()
    : m_lastsow(0)
{
    // reserve space for five subframes
    m_buffer = SubframeVector(5);
}

/**
 * @brief SubframeBuffer::addSubframe Add Subframe to buffer.
 *
 * Collects all subframes, until a message type is complete.
 *
 * Attention: If you don't fetch a completeted data set before adding a
 * new subframe, this data set will be dropped.
 *
 * @param sf Subframe to add to the buffer.
 */
void SubframeBuffer::addSubframe(const Subframe &sf)
{
    const std::size_t fraid = sf.getFrameID();
    const std::size_t pnum = sf.getPageNum();
    const uint32_t sow = sf.getSOW();

    // Last message SOW should fit the current one + 6s. So we can
    // easily detect data gaps.
    if ((m_lastsow + D1_SUBFRAME_DURATION != sow) && (m_lastsow > 0))
    {
        std::cout << "Warning: SubframeBuffer: Data gap of "
                  << sow - (m_lastsow + D1_SUBFRAME_DURATION)
                  << "s detected!" << std::endl;

        // initialize new data sets, because the old are uncompleted
        clearEphemerisData();
        clearAlmanacData();
    }
    m_lastsow = sow;

    // if we hit the beginning of frame1, start collecting ephemeris data
    if (fraid == 1 && pnum == 0 && m_buffer[0].size() > 0)
    {
        std::cout << "SubframeBuffer: Auto clear of incomplete ephemeris data set" << std::endl;
        clearEphemerisData();
    }

    // if we hit the beginning of frame4, start collecting almanac data
    // this means auto clear of old almanac data
    if (fraid == 4 && pnum == 1 && m_buffer[3].size() > 0)
    {
        std::cout << "SubframeBuffer: Auto clear of incomplete alamanac data set" << std::endl;
        clearAlmanacData();
    }

    // new Pnum has to be bigger than the previous one
//    if (m_buffer[fraid - 1].size() && m_buffer[fraid - 1].back().getPageNum() > pnum)
//    {
//        std::cout << "not bigger: " << pnum << " > " << m_buffer[fraid - 1].back().getPageNum() << std::endl;
//    }

    // if size() > D1_FRAME_SIZE...

    m_buffer[fraid - 1].push_back(sf);
}

bool SubframeBuffer::isEphemerisComplete() const
{
    return m_buffer[0].size() == D1_FRAME_SIZE[0]
            && m_buffer[1].size() == D1_FRAME_SIZE[1]
            && m_buffer[2].size() == D1_FRAME_SIZE[2];
}

// stub, D1 has no Integrity data
bool SubframeBuffer::isIntegrityComplete() const
{
    return false;
}

bool SubframeBuffer::isAlmanacComplete() const
{
    return m_buffer[3].size() == D1_FRAME_SIZE[3]
            && m_buffer[4].size() == D1_FRAME_SIZE[4];
}

SubframeVector SubframeBuffer::flushEphemerisData()
{
    SubframeVector ephdata;

    // D1: first, second and third frame contain ephemeris data
    ephdata.push_back(m_buffer[0]);
    ephdata.push_back(m_buffer[1]);
    ephdata.push_back(m_buffer[2]);

    clearEphemerisData();

    return ephdata;
}

void SubframeBuffer::clearEphemerisData()
{
    m_buffer[0].clear();
    m_buffer[1].clear();
    m_buffer[2].clear();
}

// stub
void SubframeBuffer::clearIntegrityData()
{
}

void SubframeBuffer::clearAlmanacData()
{
    m_buffer[3].clear();
    m_buffer[4].clear();
}

} // namespace bnav
