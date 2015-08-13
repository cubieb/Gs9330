#include "SystemInclude.h"
#include "Common.h"

#include "Types.h"
#include "Descriptor.h"
#include "TransportStream.h"

using namespace std;
/**********************class TransportStream**********************/
TransportStream::TransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId)
    : transportStreamId(theTransportStreamId), originalNetworkId(theOriginalNetworkId)
{
    descriptors.reset(new Descriptors);
}

void TransportStream::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    descriptors->AddDescriptor(tag, data, dataSize);
}

size_t TransportStream::GetCodesSize() const
{
    return (descriptors->GetCodesSize() + 4);
}

size_t TransportStream::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    assert(GetCodesSize() <= bufferSize);

    ptr = ptr + Write16(ptr, transportStreamId);
    ptr = ptr + Write16(ptr, originalNetworkId);

    ptr = ptr + descriptors->MakeCodes(ptr, bufferSize);

    return (ptr - buffer);
}

void TransportStream::Put(std::ostream& os) const
{
    os << "transportStreamId = " << (uint_t)transportStreamId
        << ", originalNetworkId = " << (uint_t)originalNetworkId  << endl;

    os << descriptors;
}