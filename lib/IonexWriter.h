#ifndef IONEXWRITER_H
#define IONEXWRITER_H

namespace bnav
{

class IonexWriter
{
public:
    IonexWriter();

private:
    void writeHeader(bool isGIM);
};

} // namespace bnav

#endif // IONEXWRITER_H
