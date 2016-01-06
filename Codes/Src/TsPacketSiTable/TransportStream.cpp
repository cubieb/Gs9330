#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/PacketHelper.h"
#include "Include/Foundation/Converter.h"
#include "Include/Foundation/Crc32.h"
#include "Include/Foundation/Deleter.h"

/* TsPacketSiTable */
#include "Descriptor.h"
#include "TransportStream.h"
using namespace std;

/**********************class TransportStream**********************/
TransportStream::TransportStream(TsId transportStreamId, OnId originalNetworkId)
    : transportStreamId(transportStreamId), originalNetworkId(originalNetworkId)
{
}

TransportStream::~TransportStream()
{
}

void TransportStream::AddDescriptor(Descriptor *descriptor)
{
    descriptors.AddDescriptor(descriptor);
}

size_t TransportStream::GetCodesSize() const
{
    return (descriptors.GetCodesSize() + 4);
}

OnId TransportStream::GetOnId() const
{
    return originalNetworkId;
}

TsId TransportStream::GetTsId() const
{
    return transportStreamId;
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

/**********************class TransportStreams**********************/
TransportStreams::TransportStreams()
{
}

TransportStreams::~TransportStreams()
{
    for_each(transportStreams.begin(), transportStreams.end(), ScalarDeleter<TransportStream>());
}

void TransportStreams::AddTransportStream(TsId tsId, OnId onId)
{
    TransportStream *ts = new TransportStream(tsId, onId);
    transportStreams.push_back(ts);
}

void TransportStreams::AddTsDescriptor(TsId tsId, Descriptor *descriptor)
{
    list<TransportStream*>::iterator iter;

    iter = find_if(transportStreams.begin(), transportStreams.end(), EqualTransportStream(tsId));
    assert(iter != transportStreams.end());
    (*iter)->AddDescriptor(descriptor);
}
    
size_t TransportStreams::GetCodesSize(list<TsId>& tsIds) const
{
    size_t size = 2;  //reserved_future_use + transport_stream_loop_length
    for (auto iter: transportStreams)
    {
        list<TsId>::iterator tsIdIter = find(tsIds.begin(), tsIds.end(), iter->GetTsId());
        if (tsIdIter != tsIds.end())
        {
            size = size + iter->GetCodesSize();
        }
    }

    return size;
}

size_t TransportStreams::MakeCodes(std::list<TsId>& tsIds, uchar_t *buffer, size_t bufferSize) const
{
    size_t size = GetCodesSize(tsIds);
    //reserved_future_use + transport_stream_loop_length
    uint16_t reservedFutureUse = (Reserved4Bit << 12) | size;  

    uchar_t *ptr = buffer;
    ptr = ptr + WriteBuffer(ptr, reservedFutureUse);
    for (auto iter: transportStreams)
    {
        list<TsId>::iterator tsIdIter = find(tsIds.begin(), tsIds.end(), iter->GetTsId());
        if (tsIdIter != tsIds.end())
        {
            ptr = ptr + iter->MakeCodes(ptr, buffer + bufferSize - ptr);
        }
    }

    assert(ptr - buffer == size);
    return (ptr - buffer);
}