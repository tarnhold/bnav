#include <unittest++/UnitTest++.h>
#include "TestConfig.h"

#include "DateTime.h"

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
            CHECK(dt.getDay() == 1);
            CHECK(dt.getMonth() == 1);
            CHECK(dt.getYear() == 2006);
            CHECK(dt.getHourString() == "00");
            CHECK(dt.getMinuteString() == "00");
            CHECK(dt.getSecondString() == "00");

            // BDT reference epoch with milliseconds
            dt.setWeekAndSOW(0, 0, 0);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);
            CHECK(dt.getDay() == 1);
            CHECK(dt.getMonth() == 1);
            CHECK(dt.getYear() == 2006);
            CHECK(dt.getHourString() == "00");
            CHECK(dt.getMinuteString() == "00");
            CHECK(dt.getSecondString() == "00");
        }
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::GPST);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);

            // GPST reference epoch
            dt.setWeekAndSOW(0, 0);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);
            CHECK(dt.getDay() == 6);
            CHECK(dt.getMonth() == 1);
            CHECK(dt.getYear() == 1980);
            CHECK(dt.getHourString() == "00");
            CHECK(dt.getMinuteString() == "00");
            CHECK(dt.getSecondString() == "00");
        }
        // some arbitrary date in GPST in 2014
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::GPST);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);

            dt.setWeekAndSOW(1801, 0);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);
            CHECK(dt.getDay() == 13);
            CHECK(dt.getMonth() == 7);
            CHECK(dt.getYear() == 2014);
            CHECK(dt.getHourString() == "00");
            CHECK(dt.getMinuteString() == "00");
            CHECK(dt.getSecondString() == "00");
        }
        // some arbitrary date in BDT in 2014
        {
            bnav::DateTime dt;
            dt.setTimeSystem(bnav::TimeSystem::BDT);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);

            dt.setWeekAndSOW(445, 0);
            CHECK(dt.getTimeSystem() == bnav::TimeSystem::BDT);
            CHECK(dt.getDay() == 13);
            CHECK(dt.getMonth() == 7);
            CHECK(dt.getYear() == 2014);
            CHECK(dt.getHourString() == "00");
            CHECK(dt.getMinuteString() == "00");
            CHECK(dt.getSecondString() == "00");
        }
    }

    // operator==
    TEST(testDateTime_operatorEquals)
    {
        bnav::DateTime dt;
        dt.setTimeSystem(bnav::TimeSystem::GPST);
        CHECK(dt.getTimeSystem() == bnav::TimeSystem::GPST);
        dt.setWeekAndSOW(0, 60);
        CHECK(dt.getDay() == 6);
        CHECK(dt.getMonth() == 1);
        CHECK(dt.getYear() == 1980);
        CHECK(dt.getHourString() == "00");
        CHECK(dt.getMinuteString() == "01");
        CHECK(dt.getSecondString() == "00");

        dt.setWeekAndSOW(1, 1);
        CHECK(dt.getDay() == 13);
        CHECK(dt.getMonth() == 1);
        CHECK(dt.getYear() == 1980);
        CHECK(dt.getHourString() == "00");
        CHECK(dt.getMinuteString() == "00");
        CHECK(dt.getSecondString() == "01");

        dt.setWeekAndSOW(2, 3600);
        CHECK(dt.getDay() == 20);
        CHECK(dt.getMonth() == 1);
        CHECK(dt.getYear() == 1980);
        CHECK(dt.getHourString() == "01");
        CHECK(dt.getMinuteString() == "00");
        CHECK(dt.getSecondString() == "00");
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
            CHECK(dt.getDay() == 1);
            CHECK(dt.getMonth() == 1);
            CHECK(dt.getYear() == 2006);
            CHECK(dt.getHourString() == "00");
            CHECK(dt.getMinuteString() == "00");
            CHECK(dt.getSecondString() == seconds.str());
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
            CHECK(dt.getDay() == 1);
            CHECK(dt.getMonth() == 1);
            CHECK(dt.getYear() == 2006);
            CHECK(dt.getHourString() == "00");
            CHECK(dt.getMinuteString() == minutes.str());
            CHECK(dt.getSecondString() == "00");
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
            CHECK(dt.getDay() == 1);
            CHECK(dt.getMonth() == 1);
            CHECK(dt.getYear() == 2006);
            CHECK(dt.getHourString() == hours.str());
            CHECK(dt.getMinuteString() == "00");
            CHECK(dt.getSecondString() == "00");
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
            CHECK(dt.getDay() == i + 1);
            CHECK(dt.getMonth() == 1);
            CHECK(dt.getYear() == 2006);
            CHECK(dt.getHourString() == "00");
            CHECK(dt.getMinuteString() == "00");
            CHECK(dt.getSecondString() == "00");
        }
    }
}
