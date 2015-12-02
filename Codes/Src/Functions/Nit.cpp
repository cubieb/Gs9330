#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Types.h"
#include "Crc32.h"
#include "Descriptor.h"
#include "TransportStream.h"
#include "Nit.h"

using namespace std;
/**********************class Nit**********************/
Nit::Nit(const char *key)
    : Section(key), tableId(0), networkId(0), versionNumber(0), sectionNumber(0), lastSectionNumber(0)
{
    descriptors.reset(new Descriptors);
    transportStreams.reset(new TransportStreams);
}

Nit::Nit(const char *key, uchar_t *buffer)
    : Section(key)
{    
    descriptors.reset(new Descriptors);
    transportStreams.reset(new TransportStreams);

    uchar_t *ptr = buffer;
    uint16_t sectionLength;
    ptr = ptr + Read8(ptr, tableId);
    ptr = ptr + Read16(ptr, sectionLength);
    sectionLength = sectionLength & 0xfff;
    
    ptr = ptr + Read16(ptr, networkId);
    ptr = ptr + Read8(ptr, versionNumber);
    versionNumber = (versionNumber & 0x2f) >> 1;

    ptr = ptr + Read8(ptr, sectionNumber);
    ptr = ptr + Read8(ptr, lastSectionNumber);

    uint16_t networdDescriptorLen;
    ptr = ptr + Read16(ptr, networdDescriptorLen);
    networdDescriptorLen = networdDescriptorLen & 0xfff;

    uchar_t *endPtr1, *endPtr2;
    endPtr1 = ptr + networdDescriptorLen;
    if (ptr < endPtr1)
    {        
        shared_ptr<Descriptor> descriptor(CreateDescriptor(ptr));
        if (descriptor != nullptr)
        {
            ptr = ptr + descriptor->GetCodesSize();
            descriptors->AddDescriptor(descriptor);
        }
    }
    
    uint16_t transportStreamLoopLength;
    ptr = ptr + Read16(ptr, transportStreamLoopLength);
    transportStreamLoopLength = transportStreamLoopLength & 0xfff;
    endPtr1 = ptr + transportStreamLoopLength;
    while (ptr < endPtr1)
    {
        uint16_t tsId, onId, tsDesLength;
        ptr = ptr + Read16(ptr, tsId);
        ptr = ptr + Read16(ptr, onId);
        AddTs(tsId, onId);

        ptr = ptr + Read16(ptr, tsDesLength);
        tsDesLength = tsDesLength & 0xfff;
        endPtr2 = ptr + tsDesLength;
        while (ptr < endPtr2)
        {                
            shared_ptr<Descriptor> descriptor(CreateDescriptor(ptr));
            if (descriptor != nullptr)
            {
                ptr = ptr + descriptor->GetCodesSize();
                transportStreams->AddTsDescriptor(tsId, descriptor);
            }
        }
    }
}

uint16_t Nit::GetSectionId() const
{
    return networkId;
}

uint16_t Nit::GetPid() const
{
    return Pid;
}

void Nit::SetTableId(uchar_t data)
{
    /* Table 2: Allocation of table_id values */
    assert(data == 0x40 || data == 0x41);
    tableId = data;
}

uchar_t Nit::GetTableId() const
{
    return tableId;
}

void Nit::SetNetworkId(uint16_t data)
{
    networkId = data;
}

uint16_t Nit::GetNetworkId() const
{
    return networkId;
}

void Nit::SetVersionNumber(uchar_t data)
{
    versionNumber = data;
}

void Nit::SetSectionNumber(uchar_t data)
{
    sectionNumber = data;
}

void Nit::SetLastSectionNumber(uchar_t data)
{
    lastSectionNumber = data;
}

void Nit::AddDescriptor(std::string &data)
{
    Descriptor* ptr = CreateDescriptor(data);
    if (ptr == nullptr)
    {
        errstrm << "we do not support descriptor whose tag = " 
            << hex << data[0] << data[1] << endl;
        return;
    }

    shared_ptr<Descriptor> discriptor(ptr);
    descriptors->AddDescriptor(discriptor);
}

void Nit::AddTs(uint16_t tsId, uint16_t onId)
{
    transportStreams->AddTransportStream(tsId, onId);
}

void Nit::AddTsDescriptor(uint16_t tsId, std::string &data)
{
    Descriptor* ptr = CreateDescriptor(data);
    if (ptr == nullptr)
    {
        errstrm << "we do not support descriptor whose tag = " 
            << hex << data[0] << data[1] << endl;
        return;
    }

    shared_ptr<Descriptor> discriptor(ptr);
    transportStreams->AddTsDescriptor(tsId, discriptor);
}

size_t Nit::GetCodesSize() const
{
    size_t descriptorSize = descriptors->GetCodesSize();
    size_t transportStreamSize = transportStreams->GetCodesSize();

    return (sizeof(network_information_section) + descriptorSize + transportStreamSize); 
}

size_t Nit::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    uint16_t ui16Value; 
    size_t  size = GetCodesSize();
    
    assert(size <= bufferSize && size <= (MaxNitSectionLength - 3));

    ptr = ptr + Write8(ptr, tableId);
    //section_syntax_indicator:1 + reserved_future_use1:1 + reserved1:2 + section_length:12
    ui16Value = (NitSectionSyntaxIndicator << 15) | (Reserved1Bit << 14) | (Reserved2Bit << 12) | (size - 3);

    ptr = ptr + Write16(ptr, ui16Value);  //section_length
    ptr = ptr + Write16(ptr, networkId);  //network_id
    
    /* current_next_indicator: This 1-bit indicator, when set to "1" indicates that the sub_table is the currently applicable
       sub_table. When the bit is set to "0", it indicates that the sub_table sent is not yet applicable and shall be the next
       sub_table to be valid.
     */
    uchar_t currentNextIndicator = 1;
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator);
    ptr = ptr + Write8(ptr, sectionNumber);    //section_number
    ptr = ptr + Write8(ptr, lastSectionNumber);  //last_section_number

    ptr = ptr + descriptors->MakeCodes(ptr, bufferSize);
    ptr = ptr + transportStreams->MakeCodes(ptr, bufferSize);

    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.CalculateCrc(buffer, ptr - buffer));
    return (ptr - buffer);
}

void Nit::Put(std::ostream& os) const
{
    ios::fmtflags flags = cout.flags( );

    os  << "table_id = " << showbase << hex <<(uint_t)tableId
        << ", section_length = " << dec << (uint_t)GetCodesSize() - 3
        << ", network_id = " << (uint_t)networkId
        << ", version_number = " << hex << (uint_t)versionNumber 
        << endl ;
    cout.flags(flags);

    os << *descriptors;
    os << *transportStreams;
}
