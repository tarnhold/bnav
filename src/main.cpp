#include "AsciiReader.h"

static void usage()
{
    std::cout << "usage: bnav [FILENAME]" << std::endl;
}

int main(int argc, char **argv)
{
    // require at least one parameter
    if (argc != 2)
    {
        usage();
        return 1;
    }

    // Open file and parse lines
    bnav::AsciiReader reader(argv[1], bnav::AsciiReaderType::TEXT_CONVERTED_SBF);
    if (!reader.isOpen())
        std::cerr << "Warning: Could not open file: " << argv[1] << std::endl;

    bnav::ReaderNavEntry data;
    while (reader.readLine(data))
    {
        //std::cout << "prn: " << data.getPRN() << " tow: " << data.getTOW() << std::endl;
    }
    reader.close();


    return 0;
}

