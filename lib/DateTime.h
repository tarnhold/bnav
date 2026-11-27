#ifndef DATETIME_H
#define DATETIME_H

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

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
    /// Calendar time. Empty until setWeekAndSOW / setISODateTime / setToCurrentDateTimeUTC.
    std::optional<std::chrono::sys_time<std::chrono::milliseconds>> m_time;
    uint32_t m_weeknum; ///< Safe weeknum in weeks of TimeSystem
    uint32_t m_sow; ///< Safe SOW of week

public:
    DateTime();
    DateTime(const TimeSystem ts, const std::string_view isostr);
    DateTime(const TimeSystem ts, const uint32_t weeknum, const uint32_t sow, const uint32_t millisec = 0);

    void setTimeSystem(const TimeSystem ts);
    void setToCurrentDateTimeUTC();
    void setWeekAndSOW(const uint32_t weeknum, const uint32_t sow, const uint32_t millisec = 0);
    void setISODateTime(const std::string_view isostr);

    [[nodiscard]] TimeSystem getTimeSystem() const;

    [[nodiscard]] uint32_t getWeekNum() const;
    [[nodiscard]] uint32_t getSOW() const;

    [[nodiscard]] uint32_t getDay() const;
    [[nodiscard]] uint32_t getMonth() const;
    [[nodiscard]] uint32_t getYear() const;

    [[nodiscard]] std::string getDayString() const;
    [[nodiscard]] std::string getMonthString() const;
    [[nodiscard]] std::string getYearString() const;

    [[nodiscard]] uint32_t getHour() const;
    [[nodiscard]] uint32_t getMinute() const;
    [[nodiscard]] uint32_t getSecond() const;

    [[nodiscard]] std::string getHourString() const;
    [[nodiscard]] std::string getMinuteString() const;
    [[nodiscard]] std::string getSecondString() const;

    [[nodiscard]] std::string getMonthNameShort() const;

    [[nodiscard]] std::string getISODate() const;
    [[nodiscard]] std::string getDateTimeString() const;
    [[nodiscard]] std::string getIonexDate() const;

    [[nodiscard]] bool isSameIonexDay(const DateTime &rhs) const;

    [[nodiscard]] bool operator==(const DateTime &rhs) const;
    [[nodiscard]] bool operator<(const DateTime &rhs) const;
    [[nodiscard]] bool operator>(const DateTime &rhs) const;
    /// Absolute difference, truncated to whole seconds (formerly boost posix_time total_seconds()).
    [[nodiscard]] std::chrono::seconds operator-(const DateTime &rhs) const;

private:
    [[nodiscard]] std::chrono::sys_time<std::chrono::milliseconds> timePoint() const;
};

} // namespace bnav

#endif // DATETIME_H
