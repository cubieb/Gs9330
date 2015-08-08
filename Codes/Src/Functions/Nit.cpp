#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Crc32.h"
#include "Descriptor.h"
#include "XmlDataWrapper.h"
#include "Nit.h"

using namespace std;

/**********************class NitTransportStream**********************/
NitTransportStream::NitTransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId)
    : transportStreamId(theTransportStreamId), originalNetworkId(theOriginalNetworkId)
{}

void NitTransportStream::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    DescriporFactory factory;
    shared_ptr<Discriptor> discripter(factory.Create(tag, data, dataSize));
    if (discripter != nullptr)
    {
        discripters.push_back(discripter);
    }
    else
        errstrm << "cant create descriptor, tag = " << (uint_t)tag << endl;
}

size_t NitTransportStream::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;

    ptr = ptr + Write16(ptr, transportStreamId);
    ptr = ptr + Write16(ptr, originalNetworkId);

    uchar_t *pos = ptr;
    ptr = ptr + Write16(ptr, 0);
    size_t size = 0;
    for (auto iter = discripters.begin(); iter != discripters.end(); ++iter)
    {
        ptr = ptr + (*iter)->MakeCodes(ptr, bufferSize - (ptr - buffer));
        size = size + (*iter)->GetCodesSize();
    }
    uint16_t ui16Value = (Reserved4Bit << 12) | size;
    return (ptr - buffer);
}

size_t NitTransportStream::GetCodesSize() const
{
    size_t size = 0;
    for (const auto iter: discripters)
    {
        size = size + iter->GetCodesSize();
    }
    return (size + 6);
}

void NitTransportStream::Put(std::ostream& os) const
{
    os << "transportStreamId = " << (uint_t)transportStreamId
        << ", originalNetworkId = " << (uint_t)originalNetworkId << endl;
    uint_t i = 0;
    for(auto iter: discripters)
    {
        os << "Destripter " << i++ << ": " << *iter << endl;
    }
}

/**********************class Nit**********************/
Nit::Nit(): tableId(0), networkId(0), versionNumber(0)
{}

void Nit::SetTableId(uchar_t data)
{
    tableId = data;
}

void Nit::SetNetworkId(uint16_t data)
{
    networkId = data;
}

void Nit::SetVersionNumber(uchar_t data)
{
    versionNumber = data;
}

void Nit::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    DescriporFactory factory;
    shared_ptr<Discriptor> discripter(factory.Create(tag, data, dataSize));
    if (discripter != nullptr)
    {
        discripters.push_back(discripter);
    }
    else
        errstrm << "cant create descriptor, tag = " << (uint_t)tag << endl;
}

Nit::TransportStream& Nit::AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId)
{
    shared_ptr<TransportStream> transportStream(new TransportStream(transportStreamId, transportStreamId));
    transportStreams.push_back(transportStream);
    return *transportStream;
}

size_t Nit::GetCodesSize() const
{
    size_t descriptorSize = 0, transportStreamSize = 0;
    for (const auto iter: discripters)
    {
        descriptorSize = descriptorSize + iter->GetCodesSize();
    }

    for (const auto iter: transportStreams)
    {
        transportStreamSize = transportStreamSize + iter->GetCodesSize();
    }

    return (sizeof(network_information_section) + descriptorSize + transportStreamSize);
}

size_t Nit::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    uint16_t ui16Value;
    size_t  size = GetCodesSize();
    
    assert(size <= bufferSize && size <= (MaxNitSectionLength - 3));

    ptr = ptr + Write8(ptr, tableId);
    ui16Value = (SectionSyntaxIndicator << 15) | (Reserved1Bit < 14) | (Reserved2Bit << 12);
    ui16Value = ui16Value | size;
    ptr = ptr + Write16(ptr, ui16Value);
    ptr = ptr + Write16(ptr, networkId);
    
    /* when section size greater than 1021, we should seperate the section into more htan one section.
       now we just consider the simplest case: only one section.
     */
    uchar_t currentNextIndicator = 1;
    uchar_t sectionNumber = 0;
    uchar_t lastSectionNumber = 0;
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator);
    ptr = ptr + Write8(ptr, sectionNumber);
    ptr = ptr + Write8(ptr, lastSectionNumber);  //last_section_number

    uchar_t *pos = ptr;
    ptr = ptr + Write16(ptr, 0);
    size = 0;
    for (const auto iter: discripters)
    {
        ptr = ptr + iter->MakeCodes(ptr, bufferSize - (ptr - buffer));
        size = size + iter->GetCodesSize();
    }
    ui16Value = (Reserved4Bit << 12) | size;
    Write16(pos, ui16Value);

    pos = ptr;
    ptr = ptr + Write16(ptr, 0);
    size = 0;
    for (const auto iter: transportStreams)
    {
        ptr = ptr + iter->MakeCodes(ptr, bufferSize - (ptr - buffer));
        size = size + iter->GetCodesSize();
    }
    ui16Value = (Reserved4Bit << 12) | size;
    Write16(pos, ui16Value);

    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.FullCrc(buffer, ptr - buffer));
    return (ptr - buffer);
}

void Nit::Put(std::ostream& os) const
{
    os << "tableId = " << (uint_t)tableId
        << ", networkId = " << (uint_t)networkId
        << ", versionNumber = " << (uint_t)versionNumber << endl;
    uint_t i = 0;
    for(auto iter: discripters)
    {
        os << "Destripter " << i++ << ": " << *iter << endl;
    }
    for (const auto iter: transportStreams)
    {
        os << *iter << endl;
    }
}
