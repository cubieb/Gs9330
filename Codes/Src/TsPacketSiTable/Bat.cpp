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

BatTableInterface * BatTableInterface::CreateInstance(TableId tableId, BouquetId bouquetId,  
													  Version versionNumber)
{
    if (tableId != BatTableId)
        return nullptr;

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
    //in order to packet all descriptor into single one section, we
    //demand descriptor size less than MaxBatDesAndTsContentSize.
    assert(descriptors.GetCodesSize() <= MaxBatDesAndTsContentSize);
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

size_t BatTable::GetCodesSize(TableId tableId, const TsIds &tsIds,
                              SectionNumber secIndex) const
{
    if (this->tableId != tableId)
        return 0;

    //we assume all descriptor to be packed in first section.
    assert(descriptors.GetCodesSize() <= MaxBatDesAndTsContentSize);
    //check secIndex is valid.
    assert(secIndex < GetSecNumber(tableId, tsIds));

    size_t descriptorSize = descriptors.GetCodesSize();
    size_t maxSize = MaxBatDesAndTsContentSize - descriptorSize;
    size_t tsOffset = 0;

    for (SectionNumber i = 0; i < secIndex; ++i)
    {
        transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
        descriptorSize = 0;
        maxSize = MaxBatDesAndTsContentSize;
    }
    
    size_t size = transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
    return (sizeof(bouquet_association_section) + descriptorSize + size); 
}

SiTableKey BatTable::GetKey() const
{
    return bouquetId;
}

uint_t BatTable::GetSecNumber(TableId tableId, const TsIds &tsIds) const
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

size_t BatTable::MakeCodes(TableId tableId, const TsIds &tsIds, 
						   uchar_t *buffer, size_t bufferSize,
                           SectionNumber secIndex) const
{
    uchar_t *ptr = buffer;
    size_t size = GetCodesSize(tableId, tsIds, secIndex);
    assert(size <= bufferSize);
    if (size == 0)
        return 0;
    
    //we assume all descriptor to be packed in first section.
    assert(descriptors.GetCodesSize() <= MaxBatDesAndTsContentSize);
    //check secIndex is valid.
    assert(secIndex < (SectionNumber)GetSecNumber(tableId, tsIds));

    size_t descriptorSize = descriptors.GetCodesSize();
    size_t maxSize = MaxBatDesAndTsContentSize - descriptorSize;
    size_t tsOffset = 0;

    SectionNumber secNumber = (SectionNumber)GetSecNumber(tableId, tsIds);
    for (SectionNumber i = 0; i < secIndex; ++i)
    {
        transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
        descriptorSize = 0;
        maxSize = MaxBatDesAndTsContentSize;
    }

    ptr = ptr + Write8(ptr, tableId);
    //section_syntax_indicator:1 + reserved_future_use1:1 + reserved1:2 + section_length:12
    WriteHelper<uint16_t> siHelper(ptr, ptr + 2);
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
    if (descriptorSize == 0)
    {
        ptr = ptr + Write16(ptr, (Reserved4Bit << 12) | descriptorSize); 
    }
    else
    {
        WriteHelper<uint16_t> desHelper(ptr, ptr + 2);
        //fill "reserved_future_use + bouquet_descriptors_length" to 0 temporarily.
        ptr = ptr + Write16(ptr, 0);  
        ptr = ptr + descriptors.MakeCodes(ptr, MaxBatDesAndTsContentSize);
        //rewrite reserved_future_use + network_descriptors_length.
        desHelper.Write(Reserved4Bit << 12, ptr); 
    }

    WriteHelper<uint16_t> tsHelper(ptr, ptr + 2);
    //fill "reserved_future_use + transport_stream_loop_length" to 0 temporarily.
    ptr = ptr + Write16(ptr, 0);  
    ptr = ptr + transportStreams.MakeCodes(tsIds, ptr, maxSize, tsOffset);
    //rewrite reserved_future_use + transport_stream_loop_length.
    tsHelper.Write(Reserved4Bit << 12, ptr); 
        
    siHelper.Write((BatSectionSyntaxIndicator << 15) | (Reserved1Bit << 14) | (Reserved2Bit << 12), ptr + 4); 
    ptr = ptr + Write32(ptr, Crc32::CalculateCrc(buffer, ptr - buffer));

    assert(ptr - buffer == size);
    return (ptr - buffer);
}