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
    for_each(transportStreams.begin(), transportStreams.end(), ScalarDeleter());
}

void TransportStreams::AddTransportStream(TsId tsId, OnId onId)
{
    TransportStream *ts = new TransportStream(tsId, onId);
    transportStreams.push_back(ts);
}

void TransportStreams::AddTsDescriptor(TsId tsId, Descriptor *descriptor)
{
    list<TransportStream*>::iterator iter;

    iter = find_if(transportStreams.begin(), transportStreams.end(), CompareTransportStreamId(tsId));
    assert(iter != transportStreams.end());
    (*iter)->AddDescriptor(descriptor);
}
    
size_t TransportStreams::GetCodesSize(const list<TsId>& tsIds) const
{
    size_t size = 2;  //reserved_future_use + transport_stream_loop_length
    for (auto iter: transportStreams)
    {
        list<TsId>::const_iterator tsIdIter = find(tsIds.begin(), tsIds.end(), iter->GetTsId());
        if (tsIdIter != tsIds.end())
        {
            size = size + iter->GetCodesSize();
        }
    }

    return size;
}

size_t TransportStreams::MakeCodes(const std::list<TsId>& tsIds, uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    assert(GetCodesSize(tsIds) <= bufferSize);

    ptr = ptr + Write16(ptr, 0);
    size_t size = 0;  
    for (auto iter: transportStreams)
    {
		list<TsId>::const_iterator tsIdIter = find(tsIds.begin(), tsIds.end(), iter->GetTsId());
        if (tsIdIter != tsIds.cend())
        {
            ptr = ptr + iter->MakeCodes(ptr, buffer + bufferSize - ptr);
            size = size + iter->GetCodesSize();
        }
    }
    //reserved_future_use + transport_stream_loop_length
    uint16_t ui16Value = (Reserved4Bit << 12) | size;
    Write16(buffer, ui16Value);

    assert(ptr - buffer == GetCodesSize(tsIds));
    return (ptr - buffer);
}