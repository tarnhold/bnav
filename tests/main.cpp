#include <unittest++/UnitTest++.h>

#include "AsciiReader.h"
#include "NavBits.h"

TEST(MyMath) {
    bnav::NavBits<30> my;
    my[0] = true;
    CHECK(my[0] == true);
    CHECK(my[1] == true);
}

TEST(MyAsciiReader) {
    bnav::AsciiReader reader;
    //CHECK(my.addition(3,4) == 7);
}

int main()
{
    return UnitTest::RunAllTests();
}
