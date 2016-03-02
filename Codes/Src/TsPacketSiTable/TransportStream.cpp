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
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Bat.h"
#include "Nit.h"
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
    size_t size = (descriptors.GetCodesSize() + sizeof(transport_stream));
    
    //see the definition of MaxTransportStreamSize
    assert(size <= MaxTransportStreamSize);
    return size;
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

    WriteHelper<uint16_t> desHelper(ptr, ptr + 2);
    //fill "reserved_future_use + transport_descriptors_length" to 0 temporarily.
    ptr = ptr + Write16(ptr, 0); 
    ptr = ptr + descriptors.MakeCodes(ptr, bufferSize);    
    //rewrite reserved_future_use + transport_descriptors_length.
    desHelper.Write(Reserved4Bit << 12, ptr); 

    assert(ptr - buffer == GetCodesSize());
    return (ptr - buffer);
}

/**********************class TransportStreams**********************/
TransportStreams::TransportStreams()
{
    AllocProxy();
}

TransportStreams::~TransportStreams()
{
    for_each(transportStreams.begin(), transportStreams.end(), ScalarDeleter());
    transportStreams.clear();
    FreeProxy();
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

size_t TransportStreams::GetCodesSize(size_t maxSize, size_t offset) const
{   
    size_t size = 0;
    size_t curOffset = 0;
    for (auto iter: transportStreams)
    {
        if (curOffset < offset)
        {
            curOffset = curOffset + iter->GetCodesSize();
            continue;
        }
        assert(curOffset == offset);

        if (size + iter->GetCodesSize() > maxSize)
        {
            break;
        }

        size = size + iter->GetCodesSize();
    }

    return size;
}

size_t TransportStreams::MakeCodes(uchar_t *buffer, size_t bufferSize, size_t offset) const
{
    uchar_t *ptr = buffer;

    size_t curOffset = 0;
    for (auto iter: transportStreams)
    {
        if (curOffset < offset)
        {
            curOffset = curOffset + iter->GetCodesSize();
            continue;
        }
        assert(curOffset == offset);

        if (ptr + iter->GetCodesSize() > buffer + bufferSize)
        {
            break;
        }

        ptr = ptr + iter->MakeCodes(ptr, buffer + bufferSize - ptr);
    }

    return (ptr - buffer);
}