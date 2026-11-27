#include "DateTime.h"
#include "BeiDou.h"

#include <array>
#include <cassert>
#include <charconv>
#include <chrono>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{

int parseDigits(const std::string_view text, const std::size_t pos, const std::size_t len)
{
    int value = 0;
    const char *first = text.data() + pos;
    const char *last = first + len;
    const std::from_chars_result result = std::from_chars(first, last, value);
    if (result.ec != std::errc{} || result.ptr != last)
        throw std::invalid_argument("Invalid ISO date");
    return value;
}

struct Civil
{
    int year;
    unsigned month;
    unsigned day;
    int hour;
    int minute;
    int second;
};

Civil civilFrom(const std::chrono::sys_time<std::chrono::milliseconds> tp)
{
    const auto daypoint = std::chrono::floor<std::chrono::days>(tp);
    const std::chrono::year_month_day ymd{daypoint};
    const std::chrono::hh_mm_ss<std::chrono::milliseconds> hms{tp - daypoint};
    return Civil{
        static_cast<int>(ymd.year()),
        static_cast<unsigned>(ymd.month()),
        static_cast<unsigned>(ymd.day()),
        static_cast<int>(hms.hours().count()),
        static_cast<int>(hms.minutes().count()),
        static_cast<int>(hms.seconds().count())
    };
}

// English abbreviations, matching boost::gregorian::greg_month::as_short_string().
constexpr std::array<const char *, 12> kMonthNames{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

} // namespace

namespace bnav
{

/**
 * @brief DateTime::DateTime Default constructor.
 */
DateTime::DateTime()
    : m_tsys(TimeSystem::NONE)
    , m_time(std::nullopt)
    , m_weeknum(std::numeric_limits<uint32_t>::max())
    , m_sow(std::numeric_limits<uint32_t>::max())
{
}

/**
 * @brief DateTime::DateTime Construct from ISO date string.
 */
DateTime::DateTime(const TimeSystem ts, const std::string_view isostr)
    : m_tsys(ts)
    , m_time(std::nullopt)
    , m_weeknum(std::numeric_limits<uint32_t>::max())
    , m_sow(std::numeric_limits<uint32_t>::max())
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
    , m_time(std::nullopt)
    , m_weeknum(std::numeric_limits<uint32_t>::max())
    , m_sow(std::numeric_limits<uint32_t>::max())
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
    assert(sow < std::numeric_limits<uint32_t>::max());
    assert(millisec < std::numeric_limits<uint32_t>::max());

    // safe original values, but if SOW is greater than one week, put this
    // part into weeknum.
    m_sow = sow % SECONDS_OF_A_WEEK;
    m_weeknum = weeknum + sow / SECONDS_OF_A_WEEK;
    assert(m_sow < SECONDS_OF_A_WEEK);
    assert(m_weeknum <= WEEKNUM_MAX);

    std::chrono::sys_days epoch;
    if (m_tsys == TimeSystem::BDT)
    {
        // Weeknum starts on 00:00:00 Jan, 1, 2006 BDT
        epoch = std::chrono::sys_days{std::chrono::year{2006}/std::chrono::January/1};
    }
    else if (m_tsys == TimeSystem::GPST)
    {
        // Weeknum stars on 00:00:00 Jan, 6, 1980 for GPS
        epoch = std::chrono::sys_days{std::chrono::year{1980}/std::chrono::January/6};
    }
    else
    {
        assert(false); // not implemented
        epoch = std::chrono::sys_days{};
    }

    // Cast through int, as the previous Boost implementation did, so values
    // that fit in int land on the same instant.
    m_time = epoch
            + std::chrono::weeks{static_cast<int>(weeknum)}
            + std::chrono::seconds{static_cast<int>(sow)}
            + std::chrono::milliseconds{static_cast<int>(millisec)};
}

/**
 * @brief DateTime::setISODateTime Set date and time from ISO string.
 * @param isostr String in format YYYYMMDDTHHMMSS.
 */
void DateTime::setISODateTime(const std::string_view isostr)
{
    assert(m_tsys != TimeSystem::NONE);
    // ensure ISO date YYYYMMDDTHHMMSS
    assert(isostr.length() == 15);
    assert(isostr[8] == 'T');

    const int year = parseDigits(isostr, 0, 4);
    const int month = parseDigits(isostr, 4, 2);
    const int day = parseDigits(isostr, 6, 2);
    const int hour = parseDigits(isostr, 9, 2);
    const int minute = parseDigits(isostr, 11, 2);
    const int second = parseDigits(isostr, 13, 2);

    const std::chrono::year_month_day ymd{
        std::chrono::year{year},
        std::chrono::month{static_cast<unsigned>(month)},
        std::chrono::day{static_cast<unsigned>(day)}
    };
    if (!ymd.ok())
        throw std::invalid_argument("Invalid ISO date");

    m_time = std::chrono::sys_days{ymd}
            + std::chrono::hours{hour}
            + std::chrono::minutes{minute}
            + std::chrono::seconds{second};
}

/**
 * @brief DateTime::setToCurrentDateTimeUTC Set date to current UTC time.
 */
void DateTime::setToCurrentDateTimeUTC()
{
   // one second resolution, same as boost::posix_time::second_clock
   const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

   m_tsys = TimeSystem::UTC;
   m_time = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
}

/**
 * @brief DateTime::getTimeSystem Return the current time system.
 * @return TimeSystem
 */
TimeSystem DateTime::getTimeSystem() const
{
    return m_tsys;
}

std::chrono::sys_time<std::chrono::milliseconds> DateTime::timePoint() const
{
    assert(m_time.has_value());
    return *m_time;
}

uint32_t DateTime::getWeekNum() const
{
    // at the moment it's possible that there is no weeknum set
    // see: setCurrentDateTimeUTC and setISODateTime
    assert(m_weeknum != std::numeric_limits<uint32_t>::max());
    return m_weeknum;
}

uint32_t DateTime::getSOW() const
{
    // at the moment it's possible that there is no sow set
    // see: setCurrentDateTimeUTC and setISODateTime
    assert(m_sow != std::numeric_limits<uint32_t>::max());
    return m_sow;
}

/**
 * @brief DateTime::getDay Get day number.
 * @return Day.
 */
uint32_t DateTime::getDay() const
{
    return civilFrom(timePoint()).day;
}

/**
 * @brief DateTime::getMonth Get month number.
 * @return Month.
 */
uint32_t DateTime::getMonth() const
{
    return civilFrom(timePoint()).month;
}

/**
 * @brief DateTime::getYear Get year.
 * @return Year.
 */
uint32_t DateTime::getYear() const
{
    return static_cast<uint32_t>(civilFrom(timePoint()).year);
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

uint32_t DateTime::getHour() const
{
    const int hours = civilFrom(timePoint()).hour;
    assert(hours >= 0 && hours <= 24);
    return static_cast<uint32_t>(hours);
}

/**
 * @brief DateTime::getHourString Return hours padded with zero.
 * @return Hours as string.
 */
std::string DateTime::getHourString() const
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << getHour();
    return ss.str();
}

uint32_t DateTime::getMinute() const
{
    const int minutes = civilFrom(timePoint()).minute;
    assert(minutes >= 0 && minutes <= 60);
    return static_cast<uint32_t>(minutes);
}

/**
 * @brief DateTime::getMinuteString Return minutes padded with zero.
 * @return Minutes as string.
 */
std::string DateTime::getMinuteString() const
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << getMinute();
    return ss.str();
}

uint32_t DateTime::getSecond() const
{
    const int seconds = civilFrom(timePoint()).second;
    assert(seconds >= 0 && seconds <= 60);
    return static_cast<uint32_t>(seconds);
}

/**
 * @brief DateTime::getSecondString Return seconds padded with zero.
 * @return Seconds as string.
 */
std::string DateTime::getSecondString() const
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << getSecond();
    return ss.str();
}

/**
 * @brief DateTime::getMonthNameShort Get month name as short string.
 * @return std::string month name as three chararcters.
 */
std::string DateTime::getMonthNameShort() const
{
    const uint32_t month = getMonth();
    assert(month >= 1 && month <= 12);
    return kMonthNames[month - 1];
}

std::string DateTime::getDateTimeString() const
{
    return std::string(getYearString() + "-" + getMonthString() + "-" + getDayString() + " "
                       + getHourString() + ":" + getMinuteString() + ":" + getSecondString());
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
 * @brief DateTime::isSameIonexDay Check if a given date is the same.
 *
 * Ionex: 20131201 00:00:00 till 20131202 00:00:00 is one day.
 *
 * @param rhs Date to compare to.
 * @return true if date is the same.
 */
bool DateTime::isSameIonexDay(const DateTime &rhs) const
{
    // form number YYYYMMDD, same day
    if (getYear() == rhs.getYear() && getMonth() == rhs.getMonth() && getDay() == rhs.getDay())
        return true;

    // rhs is next day 00:00, we count this to the same day, because Ionex needs
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(timePoint())};
    const std::chrono::year_month_day nextday{std::chrono::sys_days{ymd} + std::chrono::days{1}};

    if (rhs.getYear() == static_cast<uint32_t>(static_cast<int>(nextday.year()))
            && rhs.getMonth() == static_cast<unsigned>(nextday.month())
            && rhs.getDay() == static_cast<unsigned>(nextday.day()))
    {
        if (rhs.getHour() == 0 && rhs.getMinute() == 0 && rhs.getSecond() == 0)
            return true;
        else
            return false;
    }

    return false;
}

/**
 * @brief DateTime::operator== Compare two DateTime objects for equality
 * @param rhs DateTime object.
 * @return true if time system, date and time are equal.
 */
bool DateTime::operator==(const DateTime &rhs) const
{
    return (m_tsys == rhs.getTimeSystem()) && (m_time == rhs.m_time);
}

bool DateTime::operator<(const DateTime &rhs) const
{
    assert(m_tsys == rhs.getTimeSystem());
    return timePoint() < rhs.timePoint();
}

bool DateTime::operator>(const DateTime &rhs) const
{
    assert(m_tsys == rhs.getTimeSystem());
    return timePoint() > rhs.timePoint();
}

std::chrono::seconds DateTime::operator-(const DateTime &rhs) const
{
    assert(m_tsys == rhs.getTimeSystem());
    const auto lhsTime = timePoint();
    const auto rhsTime = rhs.timePoint();
    const auto diff = lhsTime < rhsTime ? (rhsTime - lhsTime) : (lhsTime - rhsTime);
    return std::chrono::duration_cast<std::chrono::seconds>(diff);
}

} // namespace bnav
