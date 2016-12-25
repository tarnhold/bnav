#include <UnitTest++/UnitTest++.h>
#include "TestConfig.h"

#include "DateTime.h"
#include "BeiDou.h"

#include <iostream>

SUITE(testDateTime)
{
    // Test empty contructor
    TEST(testDateTimeCtorEmpty)
    {
        bnav::DateTime dt;
        CHECK(dt.getTimeSystem() == bnav::TimeSystem::NONE);
    }

    // setTimeSystem
    TEST(testDateTime_setTimeSystem)
    {
        {
            bnav::DateTime dt;
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::NONE);
        }
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::BDT);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);
        }
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::GPST);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);
        }
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::UTC);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::UTC);
        }
    }

    // setWeekSOW
    TEST(testDateTime_setWeekSOW)
    {
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::BDT);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

            // BDT reference epoch
            dt.setWeekAndSOW(0, 0);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);
            CHECK_EQUAL(1, dt.getDay());
            CHECK_EQUAL(1, dt.getMonth());
            CHECK_EQUAL(2006, dt.getYear());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());

            // BDT reference epoch with milliseconds
            dt.setWeekAndSOW(0, 0, 0);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);
            CHECK_EQUAL(1, dt.getDay());
            CHECK_EQUAL(1, dt.getMonth());
            CHECK_EQUAL(2006, dt.getYear());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());
        }
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::GPST);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);

            // GPST reference epoch
            dt.setWeekAndSOW(0, 0);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);
            CHECK_EQUAL(6, dt.getDay());
            CHECK_EQUAL(1, dt.getMonth());
            CHECK_EQUAL(1980, dt.getYear());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());
        }
        // some arbitrary date in GPST in 2014
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::GPST);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);

            dt.setWeekAndSOW(1801, 0);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);
            CHECK_EQUAL(13, dt.getDay());
            CHECK_EQUAL(7, dt.getMonth());
            CHECK_EQUAL(2014, dt.getYear());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());
        }
        // some arbitrary date in BDT in 2014
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::BDT);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

            dt.setWeekAndSOW(445, 0);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);
            CHECK_EQUAL(13, dt.getDay());
            CHECK_EQUAL(7, dt.getMonth());
            CHECK_EQUAL(2014, dt.getYear());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());
        }
    }

    // setISODateTime
    TEST(testDateTime_setISODateTime)
    {
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::BDT);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

            dt.setISODateTime("19950304T123021");
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);
            CHECK_EQUAL(4, dt.getDay());
            CHECK_EQUAL(3, dt.getMonth());
            CHECK_EQUAL(1995, dt.getYear());
            CHECK_EQUAL("12", dt.getHourString());
            CHECK_EQUAL("30", dt.getMinuteString());
            CHECK_EQUAL("21", dt.getSecondString());
        }

        // set inside constructor
        {
            bnav::DateTime dt(bnav::TimeSystem::BDT, "19950304T123021");
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);
            CHECK_EQUAL(4, dt.getDay());
            CHECK_EQUAL(3, dt.getMonth());
            CHECK_EQUAL(1995, dt.getYear());
            CHECK_EQUAL("12", dt.getHourString());
            CHECK_EQUAL("30", dt.getMinuteString());
            CHECK_EQUAL("21", dt.getSecondString());
        }
    }

    // operator==
    TEST(testDateTime_operatorEquals)
    {
        bnav::DateTime dt;
        dt.setTimeSystem(bnav::TimeSystem::GPST);
        CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);
        dt.setWeekAndSOW(0, 60);
        CHECK_EQUAL(6, dt.getDay());
        CHECK_EQUAL(1, dt.getMonth());
        CHECK_EQUAL(1980, dt.getYear());
        CHECK_EQUAL("00", dt.getHourString());
        CHECK_EQUAL("01", dt.getMinuteString());
        CHECK_EQUAL("00", dt.getSecondString());

        dt.setWeekAndSOW(1, 1);
        CHECK_EQUAL(13, dt.getDay());
        CHECK_EQUAL(1, dt.getMonth());
        CHECK_EQUAL(1980, dt.getYear());
        CHECK_EQUAL("00", dt.getHourString());
        CHECK_EQUAL("00", dt.getMinuteString());
        CHECK_EQUAL("01", dt.getSecondString());

        dt.setWeekAndSOW(2, 3600);
        CHECK_EQUAL(20, dt.getDay());
        CHECK_EQUAL(1, dt.getMonth());
        CHECK_EQUAL(1980, dt.getYear());
        CHECK_EQUAL("01", dt.getHourString());
        CHECK_EQUAL("00", dt.getMinuteString());
        CHECK_EQUAL("00", dt.getSecondString());
    }

    // operator<
    TEST(testDateTime_operatorLess)
    {
        // hour
        {
            bnav::DateTime dt1(bnav::TimeSystem::GPST, "20050101T000000");
            bnav::DateTime dt2(bnav::TimeSystem::GPST, "20050101T230000");
            CHECK(dt1 < dt2);
        }

        // day
        {
            bnav::DateTime dt1(bnav::TimeSystem::GPST, "20050101T000000");
            bnav::DateTime dt2(bnav::TimeSystem::GPST, "20050102T000000");
            CHECK(dt1 < dt2);
        }

        // month
        {
            bnav::DateTime dt1(bnav::TimeSystem::GPST, "20050101T000000");
            bnav::DateTime dt2(bnav::TimeSystem::GPST, "20050201T000000");
            CHECK(dt1 < dt2);
        }

        // year
        {
            bnav::DateTime dt1(bnav::TimeSystem::GPST, "20050101T000000");
            bnav::DateTime dt2(bnav::TimeSystem::GPST, "20100101T000000");
            CHECK(dt1 < dt2);
        }
    }
}

SUITE(testDateTime_Strings)
{
    // Test getSecondString
    TEST(testDateTime_getSecondString)
    {
        bnav::DateTime dt;
        dt.setTimeSystem(bnav::TimeSystem::BDT);
        CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

        for (std::size_t i = 0; i < 60; ++i)
        {
            std::stringstream seconds;
            seconds << std::setw(2) << std::setfill('0') << i;
            dt.setWeekAndSOW(0, i);     
            CHECK_EQUAL(1, dt.getDay());
            CHECK_EQUAL(1, dt.getMonth());
            CHECK_EQUAL(2006, dt.getYear());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL(seconds.str(), dt.getSecondString());
        }
    }

    // Test getMinuteString
    TEST(testDateTime_getMinuteString)
    {
        bnav::DateTime dt;
        dt.setTimeSystem(bnav::TimeSystem::BDT);
        CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

        for (std::size_t i = 0; i < 60; ++i)
        {
            std::stringstream minutes;
            minutes << std::setw(2) << std::setfill('0') << i;
            dt.setWeekAndSOW(0, i*60);
            CHECK_EQUAL(1, dt.getDay());
            CHECK_EQUAL(1, dt.getMonth());
            CHECK_EQUAL(2006, dt.getYear());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL(minutes.str(), dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());
        }
    }

    // Test getHourString
    TEST(testDateTime_getHourString)
    {
        bnav::DateTime dt;
        dt.setTimeSystem(bnav::TimeSystem::BDT);
        CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

        for (std::size_t i = 0; i < 24; ++i)
        {
            std::stringstream hours;
            hours << std::setw(2) << std::setfill('0') << i;
            dt.setWeekAndSOW(0, i*60*60);
            CHECK_EQUAL(1, dt.getDay());
            CHECK_EQUAL(1, dt.getMonth());
            CHECK_EQUAL(2006, dt.getYear());
            CHECK_EQUAL(hours.str(), dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());
        }
    }

    // Test getDayString
    TEST(testDateTime_getDayString)
    {
        bnav::DateTime dt;
        dt.setTimeSystem(bnav::TimeSystem::BDT);
        CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

        // days 1 to 31 of January 2006
        for (std::size_t i = 1; i <= 31; ++i)
        {
            std::stringstream days;
            days << std::setw(2) << std::setfill('0') << i;
            dt.setWeekAndSOW(0, (i - 1) * bnav::SECONDS_OF_A_DAY);
            CHECK_EQUAL(i, dt.getDay());
            CHECK_EQUAL(1, dt.getMonth());
            CHECK_EQUAL(2006, dt.getYear());
            CHECK_EQUAL("2006", dt.getYearString());
            CHECK_EQUAL("01", dt.getMonthString());
            CHECK_EQUAL(days.str(), dt.getDayString());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());
        }
    }

    // Test getMonthString
    TEST(testDateTime_getMonthString)
    {
        bnav::DateTime dt;
        dt.setTimeSystem(bnav::TimeSystem::BDT);
        CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

        // months of 2006
        for (std::size_t i = 1; i <= 12; ++i)
        {
            std::stringstream month;
            month << std::setw(2) << std::setfill('0') << i;
            dt.setISODateTime("2006" + month.str() + "01T000000");
            CHECK_EQUAL(1, dt.getDay());
            CHECK_EQUAL(i, dt.getMonth());
            CHECK_EQUAL(2006, dt.getYear());
            CHECK_EQUAL("2006", dt.getYearString());
            CHECK_EQUAL(month.str(), dt.getMonthString());
            CHECK_EQUAL("01", dt.getDayString());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());
        }
    }

    // Test getYearString
    TEST(testDateTime_getYearString)
    {
        bnav::DateTime dt;
        dt.setTimeSystem(bnav::TimeSystem::BDT);
        CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

        // years 1990 to 2010
        const uint32_t offset = 1980;
        for (std::size_t i = 0; i <= 30; ++i)
        {
            std::stringstream year;
            year << std::setw(4) << std::setfill('0') << (i + offset);
            dt.setISODateTime(year.str() + "0101T000000");
            CHECK_EQUAL(1, dt.getDay());
            CHECK_EQUAL(1, dt.getMonth());
            CHECK_EQUAL(i + offset, dt.getYear());
            CHECK_EQUAL(year.str(), dt.getYearString());
            CHECK_EQUAL("01", dt.getMonthString());
            CHECK_EQUAL("01", dt.getDayString());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());
        }
    }
}

SUITE(testDateTime_Increment)
{
    // Test incrementing SOW by means of one day
    TEST(testDateTime_incrementDays)
    {
        bnav::DateTime dt;
        dt.setTimeSystem(bnav::TimeSystem::BDT);
        CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

        for (std::size_t i = 0; i < 30; ++i)
        {
            dt.setWeekAndSOW(0, i*60*60*24);
            CHECK_EQUAL(i + 1, dt.getDay());
            CHECK_EQUAL(1, dt.getMonth());
            CHECK_EQUAL(2006, dt.getYear());
            CHECK_EQUAL("00", dt.getHourString());
            CHECK_EQUAL("00", dt.getMinuteString());
            CHECK_EQUAL("00", dt.getSecondString());
        }
    }

    // Increment SOW by means of one week
    TEST(testDateTime_incrementWeeks)
    {
        bnav::DateTime dt;
        dt.setTimeSystem(bnav::TimeSystem::BDT);

        for (std::size_t i = 0; i < 10; ++i)
        {
            // internally SOW values greater than SECONDS_OF_A_WEEK are
            // reduced by this value and added to weeknum.
            // Test if weeknum increments.
            dt.setWeekAndSOW(0, i * bnav::SECONDS_OF_A_WEEK);
            CHECK_EQUAL(i, dt.getWeekNum());
            CHECK_EQUAL(0, dt.getSOW());
        }
    }
}

TEST(testDateTime_isSameIonexDay)
{
    // simple, all on the same day, only time is different
    {
        bnav::DateTime dt1(bnav::TimeSystem::BDT, "20100101T000000");
        bnav::DateTime dt2(bnav::TimeSystem::BDT, "20100101T010000");
        bnav::DateTime dt3(bnav::TimeSystem::BDT, "20100101T235959");
        CHECK(dt1.isSameIonexDay(dt2));
        CHECK(dt1.isSameIonexDay(dt3));
        CHECK(dt2.isSameIonexDay(dt1));
        CHECK(dt2.isSameIonexDay(dt3));
        CHECK(dt3.isSameIonexDay(dt1));
        CHECK(dt3.isSameIonexDay(dt2));
    }
    // case 00:00, should be same day, if day = day + 1
    {
        bnav::DateTime dt0(bnav::TimeSystem::BDT, "20091231T000000");
        bnav::DateTime dt1(bnav::TimeSystem::BDT, "20100101T000000");
        bnav::DateTime dt2(bnav::TimeSystem::BDT, "20100102T000000");
        bnav::DateTime dt3(bnav::TimeSystem::BDT, "20100102T010000");
        // year change
        CHECK(dt0.isSameIonexDay(dt1));
        CHECK(!dt1.isSameIonexDay(dt0));

        // day change
        CHECK(dt1.isSameIonexDay(dt2));
        CHECK(!dt1.isSameIonexDay(dt3));

        CHECK(dt2.isSameIonexDay(dt3));
    }
    // different dates
    {
        bnav::DateTime dt1(bnav::TimeSystem::BDT, "20110101T000000");
        bnav::DateTime dt2(bnav::TimeSystem::BDT, "20100101T000000");
        bnav::DateTime dt3(bnav::TimeSystem::BDT, "20090101T000000");
        CHECK(!dt1.isSameIonexDay(dt2));
        CHECK(!dt2.isSameIonexDay(dt1));
        CHECK(!dt1.isSameIonexDay(dt3));
        CHECK(!dt3.isSameIonexDay(dt1));
        CHECK(!dt2.isSameIonexDay(dt3));
        CHECK(!dt3.isSameIonexDay(dt2));
    }
}
