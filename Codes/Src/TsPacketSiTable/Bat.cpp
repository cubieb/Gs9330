#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/PacketHelper.h"
#include "Include/Foundation/Converter.h"
#include "Include/Foundation/Crc32.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "LengthWriteHelper.h"
#include "Bat.h"
using namespace std;

BatTableInterface * BatTableInterface::CreateInstance(TableId tableId, BouquetId bouquetId,  
													  Version versionNumber)
{
    return new BatTable(tableId, bouquetId, versionNumber);
}

/**********************class NitTable**********************/
BatTable::BatTable(TableId tableId, BouquetId bouquetId, Version versionNumber)
    : tableId(tableId), bouquetId(bouquetId), versionNumber(versionNumber)
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

size_t BatTable::GetCodesSize(TableId tableId, const std::list<TsId>& tsIds,
                              uint_t secIndex) const
{
    if (this->tableId != tableId)
        return 0;

    //we assume all descriptor to be packed in first section.
    assert(descriptors.GetCodesSize() <= MaxBatDesAndTsContentSize);
    //check secIndex is valid.
    assert(secIndex < GetSecNumber(tableId, tsIds));

    size_t desSize = descriptors.GetCodesSize();
    size_t maxSize = MaxBatDesAndTsContentSize - desSize;
    size_t tsOffset = 0;

    uint_t secNumber = GetSecNumber(tableId, tsIds);
    for (uint_t i = 0; i < secIndex; ++i)
    {
        transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
        desSize = 0;
        maxSize = MaxBatDesAndTsContentSize;
    }
    
    size_t size = transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
    return (sizeof(bouquet_association_section) + desSize + size); 
}

uint16_t BatTable::GetKey() const
{
    return bouquetId;
}

uint_t BatTable::GetSecNumber(TableId tableId, const std::list<TsId>& tsIds) const
{
    if (this->tableId != tableId)
        return 0;

    uint_t secNumber = 1;
    size_t maxSize = MaxBatDesAndTsContentSize - descriptors.GetCodesSize();
    size_t tsOffset = 0;
    transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
    
    size_t tsSize;
    maxSize = MaxBatDesAndTsContentSize;
    for (tsSize = transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
         tsSize != 0;
         tsSize = transportStreams.GetCodesSize(tsIds, maxSize, tsOffset))
    {
        ++secNumber;
    }

    return secNumber;
}

TableId BatTable::GetTableId() const
{
    return tableId;
}

size_t BatTable::MakeCodes(TableId tableId, const std::list<TsId>& tsIds, 
						   uchar_t *buffer, size_t bufferSize,
                           uint_t secIndex) const
{
    uchar_t *ptr = buffer;
    size_t size = GetCodesSize(tableId, tsIds, secIndex);
    assert(size <= bufferSize);
    if (size == 0)
        return 0;
    
    //we assume all descriptor to be packed in first section.
    assert(descriptors.GetCodesSize() <= MaxBatDesAndTsContentSize);
    //check secIndex is valid.
    assert(secIndex < GetSecNumber(tableId, tsIds));

    size_t desSize = descriptors.GetCodesSize();
    size_t maxSize = MaxBatDesAndTsContentSize - desSize;
    size_t tsOffset = 0;

    uint_t secNumber = GetSecNumber(tableId, tsIds);
    for (uint_t i = 0; i < secIndex; ++i)
    {
        transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
        desSize = 0;
        maxSize = MaxBatDesAndTsContentSize;
    }

    ptr = ptr + Write8(ptr, tableId);
    //section_syntax_indicator:1 + reserved_future_use1:1 + reserved1:2 + section_length:12
    LengthWriteHelpter<4, uint16_t> siHelper(ptr);
    //section_syntax_indicator:1 + reserved_future_use1:1 + reserved1:2 + section_length:12
    ptr = ptr + Write16(ptr, 0);  
    ptr = ptr + Write16(ptr, bouquetId);  //bouquet_id
    
    /* current_next_indicator: This 1-bit indicator, when set to "1" indicates that the sub_table 
	   is the currently applicable sub_table. When the bit is set to "0", it indicates that the 
	   sub_table sent is not yet applicable and shall be the next sub_table to be valid.
     */
    uchar_t currentNextIndicator = 1;
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator);
    ptr = ptr + Write8(ptr, secIndex);    //section_number
    ptr = ptr + Write8(ptr, secNumber - 1);  //last_section_number

    //we assume all descriptor to be packed in first section.
    if (desSize == 0)
    {
        ptr = ptr + Write16(ptr, (Reserved4Bit << 12) | desSize); 
    }
    else
    {
        LengthWriteHelpter<4, uint16_t> desHelper(ptr);
        //fill "reserved_future_use + bouquet_descriptors_length" to 0 temporarily.
        ptr = ptr + Write16(ptr, 0);  
        ptr = ptr + descriptors.MakeCodes(ptr, MaxBatDesAndTsContentSize);
        //rewrite reserved_future_use + network_descriptors_length.
        desHelper.Write(Reserved4Bit, ptr); 
    }

    LengthWriteHelpter<4, uint16_t> tsHelper(ptr);
    //fill "reserved_future_use + transport_stream_loop_length" to 0 temporarily.
    ptr = ptr + Write16(ptr, 0);  
    ptr = ptr + transportStreams.MakeCodes(tsIds, ptr, maxSize, tsOffset);
    //rewrite reserved_future_use + transport_stream_loop_length.
    tsHelper.Write(Reserved4Bit, ptr); 

    siHelper.Write((BatSectionSyntaxIndicator << 3) | (Reserved1Bit << 2) | (Reserved2Bit), ptr + 4); 
    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.CalculateCrc(buffer, ptr - buffer));

    assert(ptr - buffer == size);
    return (ptr - buffer);
}