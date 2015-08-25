#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

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

void TransportStream::AddDescriptor0x41(const std::list<std::pair<uint16_t, uchar_t>>& serviceList)
{
    descriptors->AddDescriptor0x41(serviceList);
}

void TransportStream::AddDescriptor0x44(uint32_t frequency, uint16_t fecOuter, uchar_t modulation,
                                        uint32_t symbolRate, uint32_t fecInner)
{
    descriptors->AddDescriptor0x44(frequency, fecOuter, modulation, symbolRate, fecInner);
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

    os << *descriptors;
}

/**********************class TransportStreams**********************/
void TransportStreams::AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId)
{
    auto ts = make_shared<TransportStream>(transportStreamId, originalNetworkId);
    AddComponent(ts);
}

void TransportStreams::AddTsDescriptor(uint16_t tsId, uchar_t tag, uchar_t* data, size_t dataSize)
{
    auto iter = find_if(components.begin(), components.end(), EqualTs(tsId));
    TransportStream& ts = dynamic_cast<TransportStream&>(**iter);
    ts.AddDescriptor(tag, data, dataSize);
}

void TransportStreams::AddTsDescriptor0x41(uint16_t tsId,
                                           const std::list<std::pair<uint16_t, uchar_t>>& serviceList)
{
    auto iter = find_if(components.begin(), components.end(), EqualTs(tsId));
    TransportStream& ts = dynamic_cast<TransportStream&>(**iter);
    ts.AddDescriptor0x41(serviceList);
}

void TransportStreams::AddTsDescriptor0x44(uint16_t tsId,
                                           uint32_t frequency, uint16_t fecOuter, uchar_t modulation,
                                           uint32_t symbolRate, uint32_t fecInner)
{
    auto iter = find_if(components.begin(), components.end(), EqualTs(tsId));
    TransportStream& ts = dynamic_cast<TransportStream&>(**iter);
    ts.AddDescriptor0x44(frequency, fecOuter, modulation, symbolRate, fecInner);
}

void TransportStreams::Put(std::ostream& os) const
{
    os << "reserved_future_use = " << showbase << hex << (uint_t)Reserved4Bit
        << ", transport_stream_loop_length = " << dec << GetCodesSize() - 2 << endl;
    MyBase::Put(os);
}