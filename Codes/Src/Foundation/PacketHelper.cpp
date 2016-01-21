#include "Include/Foundation/SystemInclude.h"

#include "Include/Foundation/PacketHelper.h"
using namespace std;

/******************Read from / Write to packet buffer******************/
size_t Read8(uchar_t* buf, uchar_t& value)
{
    value = buf[0];
    return sizeof(uchar_t);
}

size_t Read16(uchar_t* buf, uint16_t& value)
{
    value = 0;
    size_t size = sizeof(uint16_t);
    for (size_t i = 0; i < size; ++i)
    {
        value = (value << 8) | buf[i];
    }

    return size;
}

size_t Read32(uchar_t* buf, uint32_t& value)
{
    value = 0;
    size_t size = sizeof(uint32_t);
    for (size_t i = 0; i < size; ++i)
    {
        value = (value << 8) | buf[i];
    }

    return size;
}

size_t Read64(uchar_t* buf, uint64_t& value)
{
    value = 0;
    size_t size = sizeof(uint64_t);
    for (size_t i = 0; i < size; ++i)
    {
        value = (value << 8) | buf[i];
    }

    return size;
}

size_t Write8(uchar_t* buf, uchar_t value)
{
    buf[0] = value;
    return sizeof(uchar_t);
}

size_t Write16(uchar_t* buf, uint16_t value)
{
    size_t size = sizeof(uint16_t);
    for (size_t i = 0; i < size; ++i)
    {
        buf[size - i - 1] = value & 0xff;
        value = value >> 8;
    }
    return size;
}

size_t Write32(uchar_t* buf, uint32_t value)
{
    size_t size = sizeof(uint32_t);
    for (size_t i = 0; i < size; ++i)
    {
        buf[size - i - 1] = value & 0xff;
        value = value >> 8;
    }
    return size;
}

size_t Write64(uchar_t* buf, uint64_t value)
{
    size_t size = sizeof(uint64_t);
    for (size_t i = 0; i < size; ++i)
    {
        buf[size - i - 1] = value & 0xff;
        value = value >> 8;
    }
    return size;
}

size_t Write(void *dest, size_t destSize, const void *src, size_t count)
{
    assert(destSize >= count);
    memcpy(dest, src, count);
    return count;
}