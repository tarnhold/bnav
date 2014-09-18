#include "bnavMain.h"

#include "Benchmark.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv)
{
    Benchmark bench("Runtime");
    bench.begin();

    try
    {
        bnav::bnavMain om(argc, argv);
        om.readInputFile();
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    bench.end();

    return EXIT_SUCCESS;
}

