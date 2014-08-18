#include "SubframeBuffer.h"

// TODO: this is D1 only at the moment

namespace bnav
{

static const std::vector< std::size_t > D1_FRAME_SIZE = {1, 1, 1, 24, 24};
// one subframe has a duration of 6s
static const uint32_t D1_SUBFRAME_DURATION = 6;

SubframeBufferD1::SubframeBufferD1()
{
}

SubframeBufferD1::~SubframeBufferD1()
{
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
void SubframeBufferD1::addSubframe(const Subframe &sf)
{
    const std::size_t fraid = sf.getFrameID();
    const std::size_t pnum = sf.getPageNum();
    const uint32_t sow = sf.getSOW();

    checkLastSOW(sow, D1_SUBFRAME_DURATION);
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

    // D1: Only Subframes 4 and 5 have Pnum
    // new Pnum has to be bigger than the previous one of the same subframe
    // this proves a valid data set (continuous Pnum of a subframe), including
    // the correct length (because 24 + 1 == 24, isn't possible).
    if (fraid > 3
            && m_buffer[fraid - 1].size()
            && m_buffer[fraid - 1].back().getPageNum() + 1 != pnum)
    {
        std::cout << "Warning: SubframeBuffer: Pnum (" << pnum
                  << ") doesn't fit to previous one("
                  << m_buffer[fraid - 1].back().getPageNum()
                  << ") for Subframe " << fraid << " at SOW "
                  << sow << "!" << std::endl;
    }

    m_buffer[fraid - 1].push_back(sf);
}

/**
 * @brief SubframeBufferD1::isEphemerisComplete Check if there is a complete
 * Ephemeris data set available.
 * @return true, if complete, false, if not.
 */
bool SubframeBufferD1::isEphemerisComplete() const
{
    // complete if we have subframes 1 to 3
    return m_buffer[0].size() == D1_FRAME_SIZE[0]
            && m_buffer[1].size() == D1_FRAME_SIZE[1]
            && m_buffer[2].size() == D1_FRAME_SIZE[2];
}

/**
 * @brief SubframeBufferD1::isAlmanacComplete Check if there is a complete
 * Almanac data set is available.
 * @return true, if complete, false, if not
 */
bool SubframeBufferD1::isAlmanacComplete() const
{
    // complete if we have all pages of subframe 4 and 5
    return m_buffer[3].size() == D1_FRAME_SIZE[3]
            && m_buffer[4].size() == D1_FRAME_SIZE[4];
}

/**
 * @brief SubframeBufferD1::flushEphemerisData Get Ephemeris data and clear the
 * buffer.
 * @return Ephemeris data as SubframeBufferParam.
 */
SubframeBufferParam SubframeBufferD1::flushEphemerisData()
{
    SubframeVector ephdata;

    // D1: first, second and third frame contain ephemeris data
    for (std::size_t i = 0; i <= 2; ++i)
    {
        ephdata.push_back(m_buffer[i]);

        // ensure correct data sets, should not be possible!
        // D1 ephemeris have no Pnum
        assert(m_buffer[i].front().getPageNum() == 0);
    }

    clearEphemerisData();

    return SubframeBufferParam(SubframeBufferType::D1_EPHEMERIS, ephdata);
}

/**
 * @brief SubframeBufferD1::flushAlmanacData Get Alamanc data and clear the
 * buffer.
 * @return The Almanac data as SubframeBufferParam.
 */
SubframeBufferParam SubframeBufferD1::flushAlmanacData()
{
    SubframeVector almdata;

    for (std::size_t i = 3; i <= 4; ++i)
    {
        almdata.push_back(m_buffer[i]);

        // ensure correct data sets, should not be possible!
        assert(m_buffer[i].front().getPageNum() == 1);
        assert(m_buffer[i].back().getPageNum() == D1_FRAME_SIZE[i]);
    }

    clearAlmanacData();

    return SubframeBufferParam(SubframeBufferType::D1_ALMANAC, almdata);
}

/**
 * @brief SubframeBufferD1::clearEphemerisData Clear all Ephemeris data from the
 * buffer.
 */
void SubframeBufferD1::clearEphemerisData()
{
    m_buffer[0].clear();
    m_buffer[1].clear();
    m_buffer[2].clear();
}

/**
 * @brief SubframeBufferD1::clearAlmanacData Clear all Almanac data from the
 * buffer.
 */
void SubframeBufferD1::clearAlmanacData()
{
    m_buffer[3].clear();
    m_buffer[4].clear();
}

} // namespace bnav
