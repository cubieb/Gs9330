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

void TransportStream::AddDescriptor(std::shared_ptr<Descriptor> discriptor)
{
    descriptors->AddDescriptor(discriptor);
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

    os << *descriptors;
}

/**********************class TransportStreams**********************/
void TransportStreams::AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId)
{
    auto ts = make_shared<TransportStream>(transportStreamId, originalNetworkId);
    AddComponent(ts);
}

void TransportStreams::AddTsDescriptor(uint16_t tsId, std::shared_ptr<Descriptor> discriptor)
{
    auto iter = find_if(components.begin(), components.end(), EqualTs(tsId));
    TransportStream& ts = dynamic_cast<TransportStream&>(**iter);
    ts.AddDescriptor(discriptor);
}

void TransportStreams::Put(std::ostream& os) const
{
    os << "reserved_future_use = " << showbase << hex << (uint_t)Reserved4Bit
        << ", transport_stream_loop_length = " << dec << GetCodesSize() - 2 << endl;
    MyBase::Put(os);
}