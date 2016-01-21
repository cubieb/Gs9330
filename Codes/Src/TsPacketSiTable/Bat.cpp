#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/PacketHelper.h"
#include "Include/Foundation/Converter.h"
#include "Include/Foundation/Crc32.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Bat.h"
using namespace std;

BatTableInterface * BatTableInterface::CreateInstance(TableId tableId, BouquetId bouquetId,  Version versionNumber)
{
    return new BatTable(tableId, bouquetId, versionNumber);
}

/**********************class NitTable**********************/
BatTable::BatTable(TableId tableId, BouquetId bouquetId, Version versionNumber)
    : tableId(tableId), bouquetId(bouquetId), versionNumber(versionNumber),
      sectionNumber(0), lastSectionNumber(0)       
{
}

BatTable::~BatTable()
{
}

void BatTable::AddDescriptor(std::string &data)
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

void BatTable::AddTs(TsId tsId, OnId onId)
{
    transportStreams.AddTransportStream(tsId, onId);
}

void BatTable::AddTsDescriptor(TsId tsId, std::string &data)
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

size_t BatTable::GetCodesSize(TableId tableId, const std::list<TsId>& tsIds) const
{
    if (this->tableId != tableId)
        return 0;

    size_t descriptorSize = descriptors.GetCodesSize();
    size_t transportStreamSize = transportStreams.GetCodesSize(tsIds);

    return (sizeof(bouquet_association_section) + descriptorSize + transportStreamSize); 
}

uint16_t BatTable::GetKey() const
{
    return bouquetId;
}

TableId BatTable::GetTableId() const
{
    return tableId;
}

size_t BatTable::MakeCodes(TableId tableId, std::list<TsId>& tsIds, 
                    uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    uint16_t ui16Value; 
    size_t  size = GetCodesSize(tableId, tsIds);
    if (size == 0)
        return 0;
    
    assert(size <= bufferSize && size <= (MaxBatSectionLength - 3));

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

    ptr = ptr + descriptors.MakeCodes(ptr, bufferSize);
    ptr = ptr + transportStreams.MakeCodes(tsIds, ptr, bufferSize);

    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.CalculateCrc(buffer, ptr - buffer));

    assert(ptr - buffer == size);
    return (ptr - buffer);
}