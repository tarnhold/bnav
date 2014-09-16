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
    auto item = m_firstlast.find(sv);
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

void MessageStatistic::dump()
{
    std::cout << "Message statistic:" << std::endl;

    for (auto elem : m_count)
    {
        std::cout << std::setw(2) << elem.first.getPRN() << ": " << elem.second
                  << "\tfirst: "
                  << m_firstlast[elem.first].first.getDateTimeString()
                  << "\tlast: "
                  << m_firstlast[elem.first].second.getDateTimeString();



        std::cout << std::endl;
    }
}

} // namespace bnav
