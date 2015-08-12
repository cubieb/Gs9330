#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Descriptor.h"
#include "XmlDataWrapper.h"
#include "SectionBase.h"

using namespace std;
/**********************class SectionBase**********************/
uint16_t SectionBase::GetPid() const
{
    return 0;
}

void SectionBase::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    DescriporFactory factory;
    shared_ptr<Discriptor> discripter(factory.Create(tag, data, dataSize));
    if (discripter != nullptr)
    {
        descriptors.push_back(discripter);
    }
    else
        errstrm << "cant create descriptor, tag = " << (uint_t)tag << endl;
}

size_t SectionBase::GetCodesSize() const
{
    size_t size = 0;
    for (const auto iter: descriptors)
    {
        size = size + iter->GetCodesSize();
    }

    /* 2 bytes for reserved_future_use and transport_descriptors_length */
    return (size + 2); 
}

size_t SectionBase::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;    
    assert(SectionBase::GetCodesSize() <= bufferSize);

    ptr = ptr + Write16(ptr, 0);
    size_t size = 0;
    for (auto iter = descriptors.begin(); iter != descriptors.end(); ++iter)
    {
        ptr = ptr + (*iter)->MakeCodes(ptr, bufferSize - (ptr - buffer));
        size = size + (*iter)->GetCodesSize();
    }
    uint16_t ui16Value = (Reserved4Bit << 12) | size;
    Write16(buffer, ui16Value);

    return (ptr - buffer);
}

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

void SectionBase::Put(std::ostream& os) const
{
    uint_t i = 0;
    for(auto iter: descriptors)
    {
        os << "Destripter " << i++ << ": " << *iter << endl;
    }
}

/**********************class TransportStream**********************/
TransportStream::TransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId)
    : transportStreamId(theTransportStreamId), originalNetworkId(theOriginalNetworkId)
{}

size_t TransportStream::GetCodesSize() const
{
    return (SectionBase::GetCodesSize() + 4);
}

size_t TransportStream::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    assert(GetCodesSize() <= bufferSize);

    ptr = ptr + Write16(ptr, transportStreamId);
    ptr = ptr + Write16(ptr, originalNetworkId);

    ptr = ptr + SectionBase::MakeCodes(ptr, bufferSize);

    return (ptr - buffer);
}

void TransportStream::Put(std::ostream& os) const
{
    os << "transportStreamId = " << (uint_t)transportStreamId
        << ", originalNetworkId = " << (uint_t)originalNetworkId  << endl;

    SectionBase::Put(os);
}