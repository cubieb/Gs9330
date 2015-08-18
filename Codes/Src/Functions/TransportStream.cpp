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
    shared_ptr<TransportStream> ts(new TransportStream(transportStreamId, originalNetworkId));
    if (ts != nullptr)
    {
        AddComponent(ts);
    }
    else
        errstrm << "cant create TransportStream, tsId = " << (uint_t)transportStreamId
            << "onId = " << (uint_t)originalNetworkId << endl;
}

size_t TransportStreams::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    return MyBase::MakeCodes(buffer, bufferSize);
}

void TransportStreams::AddTsDescriptor(uint16_t tsId, uint16_t onId, uchar_t tag, uchar_t* data, size_t dataSize)
{
    for (auto iter: components)
    {        
        TransportStream& ts = dynamic_cast<TransportStream&>(*iter);
        if (ts.GetTsId() == tsId && ts.GetOnId() == onId)
        {
            ts.AddDescriptor(tag, data, dataSize);
            break;
        }
    }
}

void TransportStreams::Put(std::ostream& os) const
{
    os << "reserved_future_use = " << showbase << hex << (uint_t)Reserved4Bit
        << ", transport_stream_loop_length = " << dec << GetCodesSize() - 2 << endl;
    MyBase::Put(os);
}