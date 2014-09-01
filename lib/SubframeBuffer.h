#ifndef SUBFRAMEBUFFER_H
#define SUBFRAMEBUFFER_H

#include "BeiDou.h"
#include "Subframe.h"

#include <vector>

namespace bnav
{

using SubframeVector = std::vector< Subframe >;
using SubframeVectorVector = std::vector< std::vector< Subframe > >;

enum class SubframeBufferType
{
    D1_EPHEMERIS,
    D1_ALMANAC,
    D2_EPHEMERIS,
    D2_INTEGRITY,
    D2_ALMANAC,
    NONE
};

/// Group together Subframe data and it's type
struct SubframeBufferParam
{
    SubframeBufferType type;
    SubframeVectorVector data;

    SubframeBufferParam()
        : type(SubframeBufferType::NONE)
        , data(0)
    {
    }

    SubframeBufferParam(SubframeBufferType rtype, const SubframeVectorVector &vec)
        : type(rtype)
        , data(vec)
    {
    }
};

class SubframeBuffer
{
protected:
    // save: frame< pages >
    SubframeVectorVector m_buffer;

    uint32_t m_lastsow;

public:
    SubframeBuffer();
    virtual ~SubframeBuffer() = 0;

    virtual void addSubframe(const Subframe &sf) = 0;
    bool hasIncompleteData();

    virtual bool isEphemerisComplete() const = 0;
    //virtual bool isIntegrityComplete() const = 0;
    virtual bool isAlmanacComplete() const = 0;

    virtual SubframeBufferParam flushEphemerisData() = 0;
    //virtual SubframeBufferParam flushIntegrityData() = 0;
    virtual SubframeBufferParam flushAlmanacData() = 0;

    virtual void clearEphemerisData() = 0;
    //irtual void clearIntegrityData() = 0;
    virtual void clearAlmanacData() = 0;

protected:
    void checkLastSOW(uint32_t currentsow, uint32_t duration);
};

class SubframeBufferD1 final : public SubframeBuffer
{
public:
    SubframeBufferD1();
    ~SubframeBufferD1();

    void addSubframe(const Subframe &sf);

    bool isEphemerisComplete() const;
    bool isAlmanacComplete() const;

    SubframeBufferParam flushEphemerisData();
    SubframeBufferParam flushAlmanacData();

    void clearEphemerisData();
    void clearAlmanacData();
};

class SubframeBufferD2 final : public SubframeBuffer
{
public:
    SubframeBufferD2();
    ~SubframeBufferD2();

    void addSubframe(const Subframe &sf);

    bool isEphemerisComplete() const;
    bool isAlmanacComplete() const;

    SubframeBufferParam flushEphemerisData();
    SubframeBufferParam flushAlmanacData();

    void clearEphemerisData();
    void clearAlmanacData();
};

} // namespace bnav

#endif // SUBFRAMEBUFFER_H
