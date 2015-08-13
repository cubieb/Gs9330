#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Types.h"
#include "Crc32.h"
#include "Descriptor.h"
#include "Nit.h"

using namespace std;
/**********************class Nit**********************/
Nit::Nit(): tableId(0), networkId(0), versionNumber(0)
{
    descriptors.reset(new Descriptors);
}

uint16_t Nit::GetPid() const
{
    return Pid;
}

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
    descriptors->AddDescriptor(tag, data, dataSize);
}

Nit::TransportStream& Nit::AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId)
{
    shared_ptr<TransportStream> transportStream(new TransportStream(transportStreamId, transportStreamId));
    transportStreams.push_back(transportStream);
    return *transportStream;
}

size_t Nit::GetCodesSize() const
{
    size_t descriptorSize = descriptors->GetCodesSize();
    size_t transportStreamSize = 0;

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
    ptr = ptr + Write16(ptr, ui16Value);  //section_length
    ptr = ptr + Write16(ptr, networkId);  //network_id
    
    /* when section size greater than 1021, we should seperate the section into more htan one section.
       now we just consider the simplest case: only one section.
     */
    uchar_t currentNextIndicator = 1;
    uchar_t sectionNumber = 0;
    uchar_t lastSectionNumber = 0;
    //current_next_indicator
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator);
    ptr = ptr + Write8(ptr, sectionNumber); //section_number
    ptr = ptr + Write8(ptr, lastSectionNumber);  //last_section_number

    ptr = ptr + descriptors->MakeCodes(ptr, bufferSize);

    uchar_t *pos = ptr;
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
    assert(ptr - buffer == GetCodesSize());
    return (ptr - buffer);
}

void Nit::Put(std::ostream& os) const
{
    os  << "tableId = " << (uint_t)tableId
        << ", networkId = " << (uint_t)networkId
        << ", versionNumber = " << (uint_t)versionNumber << endl ;

    os << descriptors;

    for (const auto iter: transportStreams)
    {
        os << *iter;
    }
}
