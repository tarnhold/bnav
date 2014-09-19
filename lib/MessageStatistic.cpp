#include "MessageStatistic.h"

namespace bnav
{

/**
 * @brief MessageStatistic::MessageStatistic Simple message statistics
 *
 * Counts all messages of one SV.
 */
MessageStatistic::MessageStatistic()
    : m_count()
    , m_firstlast()
{
}

void MessageStatistic::add(const SvID &sv, const DateTime &dt)
{
    // FIXME: at the moment we ignore data gaps

    // increment message counter for SV
    ++m_count[sv];

    // keep info about first and last message date
    const auto item = m_firstlast.find(sv);
    if (item != m_firstlast.end())
    {
        // first
        if (m_firstlast[sv].first > dt)
            m_firstlast[sv].first = dt;
        // last
        if (m_firstlast[sv].second < dt)
            m_firstlast[sv].second = dt;
    }
    else
    {
        // if it's the first element, initialize
        m_firstlast[sv] = std::make_pair(dt, dt);
    }
}

void MessageStatistic::dump() const
{
    std::cout << "Message statistic:" << std::endl;

    for (const auto elem : m_count)
    {
        const auto it = m_firstlast.find(elem.first);
        assert(it != m_firstlast.end());

        std::cout << std::setw(2) << elem.first.getPRN() << ": "
                  << std::setw(6) << elem.second
                  << " first: "
                  << it->second.first.getDateTimeString()
                  << " last: "
                  << it->second.second.getDateTimeString();

        std::cout << std::endl;
    }
}

} // namespace bnav
