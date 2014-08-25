#ifndef DATETIME_H
#define DATETIME_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <stdint.h>
#include <string>

namespace bnav
{

enum class TimeSystem
{
    UTC,
    GPST,
    BDT,
    NONE
};

class DateTime
{
    TimeSystem m_tsys;
    boost::posix_time::ptime m_time; ///< Date and Time store

public:
    DateTime();
    DateTime(const TimeSystem ts, const uint32_t weeknum, const uint32_t sow, const uint32_t millisec = 0);

    void setTimeSystem(const TimeSystem ts);
    void setToCurrentDateTimeUTC();
    void setWeekAndSOW(const uint32_t weeknum, const uint32_t sow, const uint32_t millisec = 0);

    TimeSystem getTimeSystem() const;
    boost::posix_time::ptime get_ptime() const;

    uint32_t getDay() const;
    uint32_t getMonth() const;
    uint32_t getYear() const;

    std::string getHourString() const;
    std::string getMinuteString() const;
    std::string getSecondString() const;

    std::string getMonthNameShort() const;

    std::string getIonexDate() const;

    bool operator==(const DateTime &rhs) const;
};

} // namespace bnav

#endif // DATETIME_H
