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

SiTableInterface * SiTableInterface::CreateBatInstance(TableId tableId, BouquetId bouquetId,  
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
    ClearCatch();
}

void BatTable::AddTs(TsId tsId, OnId onId)
{
    transportStreams.AddTransportStream(tsId, onId);
    ClearCatch();
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
    ClearCatch();
}


SiTableKey BatTable::GetKey() const
{
    return bouquetId;
}

TableId BatTable::GetTableId() const
{
    return tableId;
}

/* protected function */
bool BatTable::CheckTableId(TableId tableId) const
{
    return (tableId == this->tableId);
}

bool BatTable::CheckTsId(TsId tsid) const
{
    return true;
}

size_t BatTable::GetFixedSize() const
{
    return BatFixedFieldSize;
}

size_t BatTable::GetVarSize() const
{
    return MaxBatDesAndTsContentSize;
}

const Descriptors& BatTable::GetVar1() const
{
    return descriptors;
}

const TransportStreams& BatTable::GetVar2(TableId tableId) const
{
    return transportStreams;
}

size_t BatTable::MakeCodes1(TableId tableId, uchar_t *buffer, size_t bufferSize, size_t var1Size,
                            SectionNumber secNumber, SectionNumber lastSecNumber) const
{
    uchar_t *ptr = buffer;

    //section_syntax_indicator:1 + reserved_future_use1:1 + reserved1:2 + section_length:12
    TableSize pseudoSize = 0;
    ptr = ptr + WriteBuffer(ptr, tableId);
    ptr = ptr + WriteBuffer(ptr, pseudoSize); 
    ptr = ptr + WriteBuffer(ptr, bouquetId);  //bouquet_id
    
    /* current_next_indicator: This 1-bit indicator, when set to "1" indicates that the sub_table 
	   is the currently applicable sub_table. When the bit is set to "0", it indicates that the 
	   sub_table sent is not yet applicable and shall be the next sub_table to be valid.
     */
    uchar_t currentNextIndicator = 1;
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator);
    /* The section_number shall be incremented by 1 with each additional section with the same
       table_id and bouquet_id.
     */
    ptr = ptr + Write8(ptr, secNumber);    //section_number
    ptr = ptr + Write8(ptr, lastSecNumber);  //last_section_number

    //we assume all descriptor to be packed in first section.
    if (var1Size == 0)
    {
        ptr = ptr + Write16(ptr, (Reserved4Bit << 12) | var1Size); 
    }
    else
    {
        WriteHelper<uint16_t> desHelper(ptr, ptr + 2);
        //fill "reserved_future_use + bouquet_descriptors_length" to 0 temporarily.
        ptr = ptr + Write16(ptr, 0);  
        ptr = ptr + descriptors.MakeCodes(ptr, var1Size);
        //rewrite reserved_future_use + network_descriptors_length.
        desHelper.Write(Reserved4Bit << 12, ptr); 
    }

    assert(ptr <= buffer + bufferSize);
    return (ptr - buffer);
}

size_t BatTable::MakeCodes2(uchar_t *buffer, size_t bufferSize,
                            size_t var2MaxSize, size_t var2Offset) const
{
    uchar_t *ptr = buffer;
    WriteHelper<uint16_t> tsHelper(ptr, ptr + 2);
    //fill "reserved_future_use + transport_stream_loop_length" to 0 temporarily.
    ptr = ptr + Write16(ptr, 0);  
    ptr = ptr + transportStreams.MakeCodes(ptr, var2MaxSize, var2Offset);
    //rewrite reserved_future_use + transport_stream_loop_length.
    tsHelper.Write(Reserved4Bit << 12, ptr); 

    assert(ptr <= buffer + bufferSize);
    return (ptr - buffer);
}

