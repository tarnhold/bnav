#include "SubframeBuffer.h"

namespace bnav
{

const std::size_t D2_FRAME_SIZE[] = {10, 6, 6, 6, 120};
// one frame has a duration of 3s (0.6s * 5)
// D2 has the same SOW for all subframes of one frame
const uint32_t D2_FRAME_DURATION = 3;

SubframeBufferD2::SubframeBufferD2()
{
}

SubframeBufferD2::~SubframeBufferD2()
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
void SubframeBufferD2::addSubframe(const Subframe &sf)
{
    const std::size_t fraid = sf.getFrameID();
    const std::size_t pnum = sf.getPageNum();
    const uint32_t sow = sf.getSOW();

    if (fraid == 1)
        checkLastSOW(sow, D2_FRAME_DURATION);
    else
    {
        // D2 has the same SOW for all subframes of one frame, check this
        if (m_lastsow != sow && m_lastsow > 0)
        {
            std::cout << "Warning: SubframeBuffer: Subframe at SOW "
                      << sow << " has another "
                      << "SOW than the current Frame!" << std::endl;

            // initialize new data sets, because the old are uncompleted
            clearEphemerisData();
            clearAlmanacData();
        }
    }
    m_lastsow = sow;

    // skip integrity data frames, this is not implemented
    // skip frame 4, as there is no data in it atm
    if (fraid > 1 && fraid < 5)
        return;

    // if we hit the beginning of frame1, start collecting ephemeris data
    if (fraid == 1 && pnum == 1 && m_buffer[0].size() > 0)
    {
        std::cout << "SubframeBuffer: Auto clear of incomplete ephemeris data set" << std::endl;
        clearEphemerisData();
    }

    // if we hit the beginning of frame4, start collecting almanac data
    // this means auto clear of old almanac data
    if (fraid == 5 && pnum == 1 && m_buffer[4].size() > 0)
    {
        std::cout << "SubframeBuffer: Auto clear of incomplete alamanac data set" << std::endl;
        clearAlmanacData();
    }

    // New Pnum has to be bigger than the previous one of the same subframe.
    // This proves a valid data set (continuous Pnum of a subframe), including
    // the correct length (because 120 + 1 == 120, isn't possible).
    if (m_buffer[fraid - 1].size() && m_buffer[fraid - 1].back().getPageNum() + 1 != pnum)
    {
        std::cout << "Warning: SubframeBuffer: Pnum (" << pnum
                  << ") doesn't fit to previous one("
                  << m_buffer[fraid - 1].back().getPageNum()
                  << ") for Subframe " << fraid << " at SOW "
                  << sow << "!" << std::endl;
    }

    m_buffer[fraid - 1].push_back(sf);
}

bool SubframeBufferD2::isEphemerisComplete() const
{
    // complete if we have all pages of subframe 1
    return m_buffer[0].size() == D2_FRAME_SIZE[0];
}

bool SubframeBufferD2::isAlmanacComplete() const
{
    // complete if we have all pages of subframe 5
    return m_buffer[4].size() == D2_FRAME_SIZE[4];
}

SubframeBufferParam SubframeBufferD2::flushEphemerisData()
{
    SubframeVectorVector ephdata;
    // D2: all ephemeris data is inside subframe 1
    ephdata.push_back(m_buffer[0]);

    // ensure correct data sets, should not be possible!
    assert(m_buffer[0].front().getPageNum() == 1);
    assert(m_buffer[0].back().getPageNum() == D2_FRAME_SIZE[0]);

    clearEphemerisData();

    return SubframeBufferParam(SubframeBufferType::D2_EPHEMERIS, ephdata);
}

SubframeBufferParam SubframeBufferD2::flushAlmanacData()
{
    SubframeVectorVector almdata;
    almdata.push_back(m_buffer[4]);

    // ensure correct data sets, should not be possible!
    assert(m_buffer[4].front().getPageNum() == 1);
    assert(m_buffer[4].back().getPageNum() == D2_FRAME_SIZE[4]);

#if 0
    // seems not neccessary at the moment, only minimal error count in sbf data
    for (auto it = m_buffer[4].begin(); it != m_buffer[4].end(); ++it)
    {
        it->checkAndFixParityAll();
    }
#endif

    clearAlmanacData();

    return SubframeBufferParam(SubframeBufferType::D2_ALMANAC, almdata);
}

void SubframeBufferD2::clearEphemerisData()
{
    m_buffer[0].clear();
}

void SubframeBufferD2::clearAlmanacData()
{
    m_buffer[4].clear();
}

} // namespace bnav
