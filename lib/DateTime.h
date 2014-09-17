#ifndef DATETIME_H
#define DATETIME_H

#include <stdint.h>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

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
    TimeSystem m_tsys; ///< Time system
    boost::posix_time::ptime m_time; ///< Date and Time store
    uint32_t m_weeknum; ///< Safe weeknum in weeks of TimeSystem
    uint32_t m_sow; ///< Safe SOW of week

public:
    DateTime();
    DateTime(const TimeSystem ts, const std::string &isostr);
    DateTime(const TimeSystem ts, const uint32_t weeknum, const uint32_t sow, const uint32_t millisec = 0);

    void setTimeSystem(const TimeSystem ts);
    void setToCurrentDateTimeUTC();
    void setWeekAndSOW(const uint32_t weeknum, const uint32_t sow, const uint32_t millisec = 0);
    void setISODateTime(const std::string &isostr);

    TimeSystem getTimeSystem() const;
    boost::posix_time::ptime get_ptime() const;

    uint32_t getWeekNum() const;
    uint32_t getSOW() const;

    uint32_t getDay() const;
    uint32_t getMonth() const;
    uint32_t getYear() const;

    std::string getDayString() const;
    std::string getMonthString() const;
    std::string getYearString() const;

    uint32_t getHour() const;
    uint32_t getMinute() const;
    uint32_t getSecond() const;

    std::string getHourString() const;
    std::string getMinuteString() const;
    std::string getSecondString() const;

    std::string getMonthNameShort() const;

    std::string getISODate() const;
    std::string getDateTimeString() const;
    std::string getIonexDate() const;

    bool isSameIonexDay(const DateTime &rhs) const;

    bool operator==(const DateTime &rhs) const;
    bool operator<(const DateTime &rhs) const;
    bool operator>(const DateTime &rhs) const;
    boost::posix_time::time_duration operator-(const DateTime &rhs) const;
};

} // namespace bnav

#endif // DATETIME_H
