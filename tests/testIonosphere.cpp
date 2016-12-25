#include <UnitTest++/UnitTest++.h>
#include "TestConfig.h"

#include "Ionosphere.h"

#include "AsciiReader.h"
#include "BeiDou.h"
#include "NavBits.h"
#include "SvID.h"

#include <iostream>

TEST(testIonoGridDimension)
{
    // default constructor
    {
        bnav::IonoGridDimension igd;
        CHECK_CLOSE(0.0, igd.latitude_north, 0.0001);
        CHECK_CLOSE(0.0, igd.latitude_south, 0.0001);
        CHECK_CLOSE(0.0, igd.latitude_spacing, 0.0001);
        CHECK_CLOSE(0.0, igd.longitude_west, 0.0001);
        CHECK_CLOSE(0.0, igd.longitude_east, 0.0001);
        CHECK_CLOSE(0.0, igd.longitude_spacing, 0.0001);
    }
    // positive values
    {
        bnav::IonoGridDimension igd(10.0, 2.0, -1.0, 0.0, 30.0, 10.0);
        CHECK_CLOSE(10.0, igd.latitude_north, 0.0001);
        CHECK_CLOSE(2.0, igd.latitude_south, 0.0001);
        CHECK_CLOSE(-1.0, igd.latitude_spacing, 0.0001);
        CHECK_CLOSE(0.0, igd.longitude_west, 0.0001);
        CHECK_CLOSE(30.0, igd.longitude_east, 0.0001);
        CHECK_CLOSE(10.0, igd.longitude_spacing, 0.0001);

        CHECK_EQUAL(9, igd.getItemCountLatitude());
        CHECK_EQUAL(4, igd.getItemCountLongitude());
    }
    // east+west and north+south
    {
        bnav::IonoGridDimension igd(1.0, -2.0, -1.0, -10.0, 30.0, 10.0);
        CHECK_CLOSE(1.0, igd.latitude_north, 0.0001);
        CHECK_CLOSE(-2.0, igd.latitude_south, 0.0001);
        CHECK_CLOSE(-1.0, igd.latitude_spacing, 0.0001);
        CHECK_CLOSE(-10.0, igd.longitude_west, 0.0001);
        CHECK_CLOSE(30.0, igd.longitude_east, 0.0001);
        CHECK_CLOSE(10.0, igd.longitude_spacing, 0.0001);

        CHECK_EQUAL(4, igd.getItemCountLatitude());
        CHECK_EQUAL(5, igd.getItemCountLongitude());
    }
    // east+west and north+south maximum
    {
        bnav::IonoGridDimension igd(90.0, -90.0, -1.0, -180.0, 180.0, 10.0);
        CHECK_CLOSE(90.0, igd.latitude_north, 0.0001);
        CHECK_CLOSE(-90.0, igd.latitude_south, 0.0001);
        CHECK_CLOSE(-1.0, igd.latitude_spacing, 0.0001);
        CHECK_CLOSE(-180.0, igd.longitude_west, 0.0001);
        CHECK_CLOSE(180.0, igd.longitude_east, 0.0001);
        CHECK_CLOSE(10.0, igd.longitude_spacing, 0.0001);

        CHECK_EQUAL(181, igd.getItemCountLatitude());
        CHECK_EQUAL(37, igd.getItemCountLongitude());
    }

    // east+west and north+south maximum, floating point spacing
    {
        bnav::IonoGridDimension igd(90.0, -90.0, -0.5, -180.0, 180.0, 2.5);
        CHECK_CLOSE(90.0, igd.latitude_north, 0.0001);
        CHECK_CLOSE(-90.0, igd.latitude_south, 0.0001);
        CHECK_CLOSE(-0.5, igd.latitude_spacing, 0.0001);
        CHECK_CLOSE(-180.0, igd.longitude_west, 0.0001);
        CHECK_CLOSE(180.0, igd.longitude_east, 0.0001);
        CHECK_CLOSE(2.5, igd.longitude_spacing, 0.0001);

        CHECK_EQUAL(361, igd.getItemCountLatitude());
        CHECK_EQUAL(145, igd.getItemCountLongitude());

        igd.latitude_spacing = -1.25;
        igd.longitude_spacing = 1.25;
        CHECK_CLOSE(-1.25, igd.latitude_spacing, 0.0001);
        CHECK_CLOSE(1.25, igd.longitude_spacing, 0.0001);

        CHECK_EQUAL(145, igd.getItemCountLatitude());
        CHECK_EQUAL(289, igd.getItemCountLongitude());
    }
}
