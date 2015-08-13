#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Types.h"
#include "Descriptor.h"
#include "XmlDataWrapper.h"
#include "SectionBase.h"

using namespace std;
/**********************class SectionBase**********************/
int SectionBase::Compare(const SectionBase& right) const
{
    size_t leftSize = GetCodesSize();
    size_t rightSize = right.GetCodesSize();

    shared_ptr<uchar_t> leftBuf(new uchar_t[leftSize], UcharDeleter());
    shared_ptr<uchar_t> rightBuf(new uchar_t[rightSize], UcharDeleter());
    MakeCodes(leftBuf.get(), leftSize);
    right.MakeCodes(rightBuf.get(), rightSize);

    int ret = memcmp(leftBuf.get(), rightBuf.get(), min(leftSize, rightSize));
    if (ret == 0)
        return ret;

    return (leftSize >  rightSize ? 1 : -1);
}

/**********************class TransportStream**********************/
TransportStream::TransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId)
    : transportStreamId(theTransportStreamId), originalNetworkId(theOriginalNetworkId)
{}

void TransportStream::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    descriptors.AddDescriptor(tag, data, dataSize);
}

size_t TransportStream::GetCodesSize() const
{
    return (descriptors.GetCodesSize() + 4);
}

size_t TransportStream::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    assert(GetCodesSize() <= bufferSize);

    ptr = ptr + Write16(ptr, transportStreamId);
    ptr = ptr + Write16(ptr, originalNetworkId);

    ptr = ptr + descriptors.MakeCodes(ptr, bufferSize);

    return (ptr - buffer);
}

void TransportStream::Put(std::ostream& os) const
{
    os << "transportStreamId = " << (uint_t)transportStreamId
        << ", originalNetworkId = " << (uint_t)originalNetworkId  << endl;

    os << descriptors;
}