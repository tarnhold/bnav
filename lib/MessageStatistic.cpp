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
{
}

void MessageStatistic::add(const SvID &sv, const DateTime &dt)
{
    // at the moment we ignore data gaps
    (void)dt;
    ++m_count[sv];
}

void MessageStatistic::dump()
{
    std::cout << "Message statistic:" << std::endl;

    for (auto elem : m_count)
        std::cout << elem.first.getPRN() << ": " << elem.second << std::endl;
}

} // namespace bnav
