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
    if (tableId != NitActualTableId && tableId != NitOtherTableId)
        return nullptr;

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

size_t NitTable::GetCodesSize(TableId tableId, const std::list<TsId>& tsIds, 
                              uint_t secIndex) const
{    
    if (this->tableId != tableId)
        return 0;

    //we assume all descriptor to be packed in first section.
    assert(descriptors.GetCodesSize() <= MaxNitDesAndTsContentSize);
    //check secIndex is valid.
    assert(secIndex < GetSecNumber(tableId, tsIds));

    size_t desSize = descriptors.GetCodesSize();
    size_t maxSize = MaxNitDesAndTsContentSize - desSize;
    size_t tsOffset = 0;
    
    uint_t secNumber = GetSecNumber(tableId, tsIds);
    for (uint_t i = 0; i < secIndex; ++i)
    {
        transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
        desSize = 0;
        maxSize = MaxNitDesAndTsContentSize;
    }
    
    size_t size = transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
    return (sizeof(network_information_section) + desSize + size); 
}

SiTableKey NitTable::GetKey() const
{
    return networkId;
}

uint_t NitTable::GetSecNumber(TableId tableId, const std::list<TsId>& tsIds) const
{
    if (this->tableId != tableId)
        return 0;

    uint_t secNumber = 1;
    size_t maxSize = MaxNitDesAndTsContentSize - descriptors.GetCodesSize();
    size_t tsOffset = 0;
    transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
    
    size_t tsSize;
    maxSize = MaxNitDesAndTsContentSize;
    for (tsSize = transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
         tsSize != 0;
         tsSize = transportStreams.GetCodesSize(tsIds, maxSize, tsOffset))
    {
        ++secNumber;
    }

    return secNumber;
}

TableId NitTable::GetTableId() const
{
    return tableId;
}

size_t NitTable::MakeCodes(TableId tableId, const std::list<TsId>& tsIds, 
                           uchar_t *buffer, size_t bufferSize, 
                           uint_t secIndex) const
{
    uchar_t *ptr = buffer;
    size_t size = GetCodesSize(tableId, tsIds, secIndex);
    assert(size <= bufferSize);
    if (size == 0)
        return 0;

    //we assume all descriptor to be packed in first section.
    assert(descriptors.GetCodesSize() <= MaxNitDesAndTsContentSize);
    //check secIndex is valid.
    assert(secIndex < GetSecNumber(tableId, tsIds));

    size_t desSize = descriptors.GetCodesSize();
    size_t maxSize = MaxNitDesAndTsContentSize - desSize;
    size_t tsOffset = 0;
    
    uint_t secNumber = GetSecNumber(tableId, tsIds);
    for (uint_t i = 0; i < secIndex; ++i)
    {
        transportStreams.GetCodesSize(tsIds, maxSize, tsOffset);
        desSize = 0;
        maxSize = MaxNitDesAndTsContentSize;
    }

    ptr = ptr + Write8(ptr, tableId);
    WriteHelper<uint16_t> siHelper(ptr, ptr+2);
    //section_syntax_indicator:1 + reserved_future_use1:1 + reserved1:2 + section_length:12
    ptr = ptr + Write16(ptr, 0);  
    ptr = ptr + Write16(ptr, networkId);  //network_id

    /* current_next_indicator: This 1-bit indicator, when set to "1" indicates that the sub_table is the 
    currently applicable sub_table. When the bit is set to "0", it indicates that the sub_table sent 
    is not yet applicable and shall be the next sub_table to be valid.
    */
    uchar_t currentNextIndicator = 1;
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator);
    ptr = ptr + Write8(ptr, secIndex);     //section_number
    ptr = ptr + Write8(ptr, secNumber - 1); //last_section_number

    //we assume all descriptor to be packed in first section.
    if (desSize == 0)
    {
        ptr = ptr + Write16(ptr, (Reserved4Bit << 12) | desSize); 
    }
    else
    {
        WriteHelper<uint16_t> desHelper(ptr, ptr + 2);
        //fill "reserved_future_use + network_descriptors_length" to 0 temporarily.
        ptr = ptr + Write16(ptr, 0);  
        ptr = ptr + descriptors.MakeCodes(ptr, MaxNitDesAndTsContentSize);
        //rewrite reserved_future_use + network_descriptors_length.
        desHelper.Write(Reserved4Bit << 12, ptr); 
    }

    WriteHelper<uint16_t> tsHelper(ptr, ptr + 2);
    //fill "reserved_future_use + transport_stream_loop_length" to 0 temporarily.
    ptr = ptr + Write16(ptr, 0);  
    ptr = ptr + transportStreams.MakeCodes(tsIds, ptr, maxSize, tsOffset);
    //rewrite reserved_future_use + transport_stream_loop_length.
    tsHelper.Write(Reserved4Bit << 12, ptr); 

    siHelper.Write((NitSectionSyntaxIndicator << 15) | (Reserved1Bit << 14) | (Reserved2Bit << 12), ptr + 4); 
    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.CalculateCrc(buffer, ptr - buffer));

    assert(size == ptr - buffer);
    return (ptr - buffer);
}