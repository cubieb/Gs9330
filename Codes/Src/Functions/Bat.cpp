#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Crc32.h"
#include "Descriptor.h"
#include "TransportStream.h"
#include "Bat.h"

using namespace std;

/**********************class Bat**********************/
Bat::Bat(): tableId(0), versionNumber(0), sectionNumber(0), lastSectionNumber(0)
{
    descriptors.reset(new Descriptors);
    transportStreams.reset(new TransportStreams);
}

uint16_t Bat::GetPid()  const
{
    return Pid;
}

uint16_t Bat::GetClassId() const
{
    return ClassId;
}

void Bat::SetTableId(uchar_t data)
{
    /* Table 2: Allocation of table_id values */
    assert(data == 0x4A);
    tableId = data;
}

void Bat::SetBouquetId(uint16_t data)
{
    bouquetId = data;
}

void Bat::SetVersionNumber(uchar_t data)
{
    versionNumber = data;
}

void Bat::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    descriptors->AddDescriptor(tag, data, dataSize);
}

void Bat::AddTs(uint16_t tsId, uint16_t onId)
{
    transportStreams->AddTransportStream(tsId, onId);
}

void Bat::AddTsDescriptor(uint16_t tsId, uchar_t tag, uchar_t* data, size_t dataSize)
{
    transportStreams->AddTsDescriptor(tsId, tag, data, dataSize);
}

void Bat::AddTsDescriptor0x41(uint16_t tsId,
                              const std::list<std::pair<uint16_t, uchar_t>>& serviceList)
{
    transportStreams->AddTsDescriptor0x41(tsId, serviceList);
}

size_t Bat::GetCodesSize() const
{
    size_t descriptorSize = descriptors->GetCodesSize();
    size_t transportStreamSize = transportStreams->GetCodesSize();

    return (sizeof(bouquet_association_section) + descriptorSize + transportStreamSize); 
}

size_t Bat::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    uint16_t ui16Value; 
    size_t  size = GetCodesSize();
    
    assert(size <= bufferSize && size <= (MaxNitSectionLength - 3));

    ptr = ptr + Write8(ptr, tableId);
    //section_syntax_indicator:1 + reserved_future_use1:1 + reserved1:2 + section_length:12
    ui16Value = (BatSectionSyntaxIndicator << 15) | (Reserved1Bit << 14) | (Reserved2Bit << 12) | (size - 3);

    ptr = ptr + Write16(ptr, ui16Value);  //section_length
    ptr = ptr + Write16(ptr, bouquetId);  //bouquet_id
    
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

    assert(ptr - buffer == size);
    return (ptr - buffer);
}

/* the following function is provided just for debug */
void Bat::Put(std::ostream& os) const
{    
    ios::fmtflags flags = cout.flags( );

    os  << "table_id = " << showbase << hex <<(uint_t)tableId
        << ", section_length = " << dec << (uint_t)GetCodesSize() - 3
        << ", bouquet_id = " << (uint_t)bouquetId
        << ", version_number = " << hex << (uint_t)versionNumber 
        << endl ;
    cout.flags(flags);

    os << *descriptors;
    os << *transportStreams;
}