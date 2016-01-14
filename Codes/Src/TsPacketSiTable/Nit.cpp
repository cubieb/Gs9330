#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/PacketHelper.h"
#include "Include/Foundation/Converter.h"
#include "Include/Foundation/Crc32.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Nit.h"
using namespace std;

NitTableInterface * NitTableInterface::CreateInstance(TableId tableId, NetId networkId, Version versionNumber)
{
    return new NitTable(tableId, networkId, versionNumber);
}

/**********************class NitTable**********************/
NitTable::NitTable(TableId tableId, NetId networkId, Version versionNumber)
    : tableId(tableId), networkId(networkId), versionNumber(versionNumber), 
      sectionNumber(0), lastSectionNumber(0)
{
}

NitTable::~NitTable()
{
}    

void NitTable::AddDescriptor(std::string &data)
{
    Descriptor* descriptor = CreateDescriptor(data);
    if (descriptor == nullptr)
    {
        errstrm << "we do not support descriptor whose tag = " 
            << hex << data[0] << data[1] << endl;
        return;
    }

    descriptors.AddDescriptor(descriptor);
}

void NitTable::AddTs(TsId tsId, OnId onId)
{
    transportStreams.AddTransportStream(tsId, onId);
}

void NitTable::AddTsDescriptor(TsId tsId, std::string &data)
{
    Descriptor* descriptor = CreateDescriptor(data);
    if (descriptor == nullptr)
    {
        errstrm << "we do not support descriptor whose tag = " 
            << hex << data[0] << data[1] << endl;
        return;
    }

    transportStreams.AddTsDescriptor(tsId, descriptor);
}

size_t NitTable::GetCodesSize(TableId tableId, const std::list<TsId>& tsIds) const
{    
    if (this->tableId != tableId)
        return 0;

    size_t descriptorSize = descriptors.GetCodesSize();
    size_t transportStreamSize = transportStreams.GetCodesSize(tsIds);

    return (sizeof(network_information_section) + descriptorSize + transportStreamSize); 
}

uint16_t NitTable::GetKey() const
{
    return networkId;
}

TableId NitTable::GetTableId() const
{
    return tableId;
}

size_t NitTable::MakeCodes(TableId tableId, std::list<TsId>& tsIds, 
                           uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    uint16_t ui16Value; 
    size_t  size = GetCodesSize(tableId, tsIds);
    if (size == 0)
        return 0;
    
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
    ptr = ptr + Write8(ptr, sectionNumber);     //section_number
    ptr = ptr + Write8(ptr, lastSectionNumber); //last_section_number

    ptr = ptr + descriptors.MakeCodes(ptr, bufferSize);
    ptr = ptr + transportStreams.MakeCodes(tsIds, ptr, bufferSize);
        
    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.CalculateCrc(buffer, ptr - buffer));

    assert(ptr - buffer == size);
    return (ptr - buffer);
}