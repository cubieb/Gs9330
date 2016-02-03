#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/PacketHelper.h"

#include "DescriptorHelper.h"
using namespace std;

CxxBeginNameSpace(UnitTest)

string GetDescriptorString(size_t size)
{
    assert(size >= 2 && size <= UCHAR_MAX);
    uchar_t buffer[UCHAR_MAX * 2 + 1] = {0};
    uchar_t *ptr = buffer;
    ptr = ptr + Write(ptr, 2, "FE", 2);
    ptr = ptr + sprintf((char *)ptr, "%02x", size - 2);

    for (size_t i = 0; i < size - 2; ++i)
    {
        ptr = ptr + sprintf((char *)ptr, "%02d", i % 100);
    }

    return string((char *)buffer);
}

CxxEndNameSpace