#include "bnavMain.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv)
{
    try
    {
        bnav::bnavMain om(argc, argv);
        om.readInputFile();
    }
    catch (std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

