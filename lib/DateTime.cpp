#include "BeiDou.h"
#include "DateTime.h"

#include <iostream>

namespace bnav
{

/**
 * @brief DateTime::DateTime Default constructor.
 */
DateTime::DateTime()
    : m_tsys(TimeSystem::NONE)
    , m_time(boost::posix_time::ptime())
    , m_weeknum(UINT32_MAX)
    , m_sow(UINT32_MAX)
{
}

/**
 * @brief DateTime::DateTime Construct from ISO date string.
 */
DateTime::DateTime(const std::string &isostr)
    : DateTime()
{
    setISODateTime(isostr);
}

/**
 * @brief DateTime::DateTime Set date and time from weeknum and SOw.
 * @param ts TimeSystem, to which weeknum, SOW and millisec refer.
 * @param weeknum Weeks since reference epoch.
 * @param sow Seconds of current week.
 * @param millisec Milliseconds part.
 */
DateTime::DateTime(const TimeSystem ts, const uint32_t weeknum, const uint32_t sow, const uint32_t millisec)
    : m_tsys(ts)
    , m_time()
    , m_weeknum(UINT32_MAX)
    , m_sow(UINT32_MAX)
{
    setWeekAndSOW(weeknum, sow, millisec);
}

/**
 * @brief DateTime::setTimeSystem Set time system
 * @param ts TimeSystem.
 */
void DateTime::setTimeSystem(const TimeSystem ts)
{
    // we don't want to mangle with TimeSystem changes...
    assert(m_tsys == TimeSystem::NONE);
    m_tsys = ts;
}

/**
 * @brief DateTime::setWeekAndSOW Set date by weeknum and SOW.
 * @param weeknum Week since reference epoch.
 * @param sow Seconds of current week.
 * @param millisec Milliseconds part.
 */
void DateTime::setWeekAndSOW(const uint32_t weeknum, const uint32_t sow, const uint32_t millisec)
{
    assert(weeknum <= WEEKNUM_MAX);
    // lets allow SOW values greater than SECONDS_OF_A_WEEK to give some comfort
    // use int32 max instead, because we cast down to int. This means we could
    // increment weeks only by SOW for 7101 weeks. This should be enough.
    assert(sow < INT32_MAX);
    assert(millisec < INT32_MAX);
    boost::gregorian::date d0;

    // safe original values, too
    m_weeknum = weeknum;
    m_sow = sow;

    if (m_tsys == TimeSystem::BDT)
    {
        // Weeknum starts on 00:00:00 Jan, 1, 2006 BDT
        d0 = boost::gregorian::date(2006, 1, 1);
    }
    else if (m_tsys == TimeSystem::GPST)
    {
        // Weeknum stars on 00:00:00 Jan, 6, 1980 for GPS
        d0 = boost::gregorian::date(1980, 1, 6);
    }
    else
    {
        assert(false); // not implemented
    }

    boost::posix_time::ptime t0(d0, boost::posix_time::hours(0));

    m_time = t0 + boost::gregorian::weeks(static_cast<int>(weeknum))
            + boost::posix_time::seconds(static_cast<int>(sow))
            + boost::posix_time::milliseconds(static_cast<int>(millisec));
}

/**
 * @brief DateTime::setISODateTime Set date and time from ISO string.
 * @param isostr String in format YYYYMMDDTHHMMSS.
 */
void DateTime::setISODateTime(const std::string &isostr)
{
    // ensure ISO date YYYYMMDDTHHMMSS
    assert(isostr.length() == 15);
    assert(isostr[8] == 'T');
    boost::posix_time::ptime date { boost::posix_time::from_iso_string(isostr) };

    m_tsys = TimeSystem::UTC;
    m_time = date;
}

/**
 * @brief DateTime::setToCurrentDateTimeUTC Set date to current UTC time.
 */
void DateTime::setToCurrentDateTimeUTC()
{
   // get the current time from the clock -- one second resolution
   boost::posix_time::ptime now { boost::posix_time::second_clock::universal_time() };

   m_tsys = TimeSystem::UTC;
   m_time = now;
}

/**
 * @brief DateTime::getTimeSystem Return the current time system.
 * @return TimeSystem
 */
TimeSystem DateTime::getTimeSystem() const
{
    return m_tsys;
}

/**
 * @brief DateTime::get_ptime Return raw ptime object.
 * @return ptime.
 */
boost::posix_time::ptime DateTime::get_ptime() const
{
    return m_time;
}

uint32_t DateTime::getWeekNum() const
{
    return m_weeknum;
}

uint32_t DateTime::getSOW() const
{
    return m_sow;
}

/**
 * @brief DateTime::getDay Get day number.
 * @return Day.
 */
uint32_t DateTime::getDay() const
{
    boost::gregorian::date d { m_time.date() };
    return d.day();
}

/**
 * @brief DateTime::getMonth Get month number.
 * @return Month.
 */
uint32_t DateTime::getMonth() const
{
    boost::gregorian::date d { m_time.date() };
    return d.month();
}

/**
 * @brief DateTime::getYear Get year.
 * @return Year.
 */
uint32_t DateTime::getYear() const
{
    boost::gregorian::date d { m_time.date() };
    return d.year();
}

/**
 * @brief DateTime::getDayString Return day padded with zero.
 * @return Day as string.
 */
std::string DateTime::getDayString() const
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << getDay();
    return ss.str();
}

/**
 * @brief DateTime::getMonthString Return month padded with zero.
 * @return Month as string.
 */
std::string DateTime::getMonthString() const
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << getMonth();
    return ss.str();
}

/**
 * @brief DateTime::getYearString Return year padded with zero.
 * @return Year as string.
 */
std::string DateTime::getYearString() const
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << getYear();
    return ss.str();
}

/**
 * @brief DateTime::getISODate Return ISO date YYYYMMDD
 * @return ISO date as string.
 */
std::string DateTime::getISODate() const
{
    return std::string(getYearString() + getMonthString() + getDayString());
}

/**
 * @brief DateTime::getHourString Return hours padded with zero.
 * @return Hours as string.
 */
std::string DateTime::getHourString() const
{
    boost::posix_time::time_duration t { m_time.time_of_day() };
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << t.hours();
    return ss.str();
}

/**
 * @brief DateTime::getMinuteString Return minutes padded with zero.
 * @return Minutes as string.
 */
std::string DateTime::getMinuteString() const
{
    boost::posix_time::time_duration t { m_time.time_of_day() };
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << t.minutes();
    return ss.str();
}

/**
 * @brief DateTime::getSecondString Return seconds padded with zero.
 * @return Seconds as string.
 */
std::string DateTime::getSecondString() const
{
    boost::posix_time::time_duration t { m_time.time_of_day() };
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << t.seconds();
    return ss.str();
}

/**
 * @brief DateTime::getMonthNameShort Get month name as short string.
 * @return std::string month name as three chararcters.
 */
std::string DateTime::getMonthNameShort() const
{
    boost::gregorian::date d { m_time.date() };
    return d.month().as_short_string();
}

/**
 * @brief DateTime::getIonexDate Convert date to Ionex format DD-MMM-YY HH:SS
 * @return Date string.
 */
std::string DateTime::getIonexDate() const
{
    std::stringstream ss;
    ss << std::to_string(getDay()) << "-"
       << getMonthNameShort() << "-"
       << std::to_string(getYear()).substr(2) << " "
       << getHourString() << ":"
       << getMinuteString();
    return ss.str();
}

/**
 * @brief DateTime::operator== Compare two DateTime objects for equality
 * @param rhs DateTime object.
 * @return true if time system, date and time are equal.
 */
bool DateTime::operator==(const DateTime &rhs) const
{
    return (m_tsys == rhs.getTimeSystem()) && (m_time == rhs.get_ptime());
}

} // namespace bnav
