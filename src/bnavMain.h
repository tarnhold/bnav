#ifndef BNAVMAIN_H
#define BNAVMAIN_H

#include "AsciiReader.h"
#include "IonosphereStore.h"
#include "SubframeBufferStore.h"

#include <string>

namespace bnav
{

class bnavMain
{
    std::string filenameInput;
    bnav::AsciiReaderType filetypeInput;
    std::string filenameIonexKlobuchar;
    std::string filenameIonexRegional;

    bnav::SubframeBufferStore sbstore;
    bnav::IonosphereStore ionostore;
    bnav::IonosphereStore ionostoreKlobuchar;

public:
    bnavMain(int argc, char *argv[]);

    void readInputFile();

private:
    void writeIonexFile(const std::string &filename);
};

} // namespace bnav

#endif // BNAVMAIN_H
