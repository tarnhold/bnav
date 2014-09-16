#ifndef MESSAGESTATISTIC_H
#define MESSAGESTATISTIC_H

#include "SvID.h"
#include "DateTime.h"

#include <map>

namespace bnav
{

class MessageStatistic
{
    std::map< SvID, uint32_t > m_count;
    std::map< SvID, std::pair<DateTime, DateTime> > m_firstlast;

public:
    MessageStatistic();

    void add(const SvID &sv, const DateTime &dt);

    void dump();
};

} // namespace bnav

#endif // MESSAGESTATISTIC_H
