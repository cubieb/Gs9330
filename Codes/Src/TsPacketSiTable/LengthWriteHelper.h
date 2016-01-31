#ifndef _LengthWriteHelper_h_
#define _LengthWriteHelper_h_

#include "Include/Foundation/SystemInclude.h"
#include "Include/Foundation/PacketHelper.h"

template <uint_t RevservedBits, typename Type>
class LengthWriteHelpter
{
public:
    LengthWriteHelpter(uchar_t *start)
        : start(start)
    {}

    size_t Write(Type reservedBits, uchar_t *end)
    {
        size_t length = end - start - sizeof(Type);
        uint_t leftShift =  sizeof(Type) * 8 - RevservedBits;
        Type value = (Type)((reservedBits << leftShift) | length);
        return WriteBuffer(start, value);
    }

private:
    uchar_t *start;
};

#endif