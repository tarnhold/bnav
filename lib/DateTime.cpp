#include "DateTime.h"

#include <iostream>

namespace bnav
{

DateTime::DateTime()
{
}

/**
 * @brief DateTime::setToCurrentDateTimeUTC Set date to current UTC time.
 */
void DateTime::setToCurrentDateTimeUTC()
{
   // get the current time from the clock -- one second resolution
   boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();

   m_time = now;
}

uint32_t DateTime::getDay() const
{
    boost::gregorian::date d = m_time.date();
    return d.day();
}

uint32_t DateTime::getMonth() const
{
    boost::gregorian::date d = m_time.date();
    return d.month();
}

uint32_t DateTime::getYear() const
{
    boost::gregorian::date d = m_time.date();
    return d.year();
}

/**
 * @brief DateTime::getMonthNameShort Get month name as short string.
 * @return std::string month name as three chararcters.
 */
std::string DateTime::getMonthNameShort() const
{
    boost::gregorian::date d = m_time.date();
    return d.month().as_short_string();
}

/**
 * @brief DateTime::getIonexDate Convert date to Ionex format DD-MMM-YY HH:SS
 * @return Date string.
 */
std::string DateTime::getIonexDate() const
{
    boost::posix_time::time_duration t = m_time.time_of_day();

    std::stringstream ss;
    ss << std::to_string(getDay()) << "-"
       << getMonthNameShort() << "-"
       << std::to_string(getYear()).substr(2) << " "
       << std::to_string(t.hours()) << ":"
       << std::to_string(t.minutes());
    return ss.str();
}

} // namespace bnav
