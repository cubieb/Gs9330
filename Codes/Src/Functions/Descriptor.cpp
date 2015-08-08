#include "SystemInclude.h"
#include "Common.h"

#include "Descriptor.h"

/**********************class UcharDescriptor**********************/
size_t UcharDescriptor::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    assert(dataSize <= bufferSize);
    memcpy(buffer, data.get(), dataSize);
    return dataSize;
}

size_t UcharDescriptor::GetCodesSize() const
{
    return dataSize;
}

void UcharDescriptor::Put(std::ostream& os) const
{
    os << (char*)data.get();
}


