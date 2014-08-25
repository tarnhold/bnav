#ifndef DATETIME_H
#define DATETIME_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <stdint.h>
#include <string>

namespace bnav
{

class DateTime
{
    boost::posix_time::ptime m_time; ///< Date and Time store

public:
    DateTime();
    DateTime(const uint32_t weeknum, const uint32_t sow);

    void setToCurrentDateTimeUTC();

    uint32_t getDay() const;
    uint32_t getMonth() const;
    uint32_t getYear() const;

    std::string getMonthNameShort() const;

    std::string getIonexDate() const;
};

} // namespace bnav

#endif // DATETIME_H
