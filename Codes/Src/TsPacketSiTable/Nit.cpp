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

SiTableInterface * SiTableInterface::CreateNitInstance(TableId tableId, NetId networkId, Version versionNumber)
{
    if (tableId != NitActualTableId && tableId != NitOtherTableId)
        return nullptr;

    return new NitTable(tableId, networkId, versionNumber);
}

/**********************class NitTable**********************/
NitTable::NitTable(TableId tableId, NetId networkId, Version versionNumber)
    : tableId(tableId), networkId(networkId), versionNumber(versionNumber)
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
    ClearCatch();
}

void NitTable::AddTs(TsId tsId, OnId onId)
{
    transportStreams.AddTransportStream(tsId, onId);
    ClearCatch();
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
    ClearCatch();
}

SiTableKey NitTable::GetKey() const
{
    return networkId;
}

TableId NitTable::GetTableId() const
{
    return tableId;
}

/* protected function */
bool NitTable::CheckTableId(TableId tableId) const
{
    return (tableId == this->tableId);
}

bool NitTable::CheckTsId(TsId tsid) const
{
    return true;
}

size_t NitTable::GetFixedSize() const
{
    return NitFixedFieldSize;
}

size_t NitTable::GetVarSize() const
{
    return MaxNitDesAndTsContentSize;
}

const Descriptors& NitTable::GetVar1() const
{
    return descriptors;
}

const TransportStreams& NitTable::GetVar2(TableId tableId) const
{
    return transportStreams;
}

size_t NitTable::MakeCodes1(TableId tableId, uchar_t *buffer, size_t bufferSize, size_t var1Size,
                            SectionNumber secNumber, SectionNumber lastSecNumber) const
{
    uchar_t *ptr = buffer;

    //section_syntax_indicator:1 + reserved_future_use1:1 + reserved1:2 + section_length:12
    TableSize pseudoSize = 0;
    ptr = ptr + WriteBuffer(ptr, tableId);
    ptr = ptr + WriteBuffer(ptr, pseudoSize); 
    ptr = ptr + WriteBuffer(ptr, networkId);  //network_id

    /* current_next_indicator: This 1-bit indicator, when set to "1" indicates that the sub_table is the 
    currently applicable sub_table. When the bit is set to "0", it indicates that the sub_table sent 
    is not yet applicable and shall be the next sub_table to be valid.
    */
    uchar_t currentNextIndicator = 1;
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator);
    /* The section_number shall be incremented by 1 with each additional section with the same
       table_id and network_id. 
     */
    ptr = ptr + WriteBuffer(ptr, secNumber);     //section_number
    ptr = ptr + WriteBuffer(ptr, lastSecNumber); //last_section_number    

    //we assume all descriptor to be packed in first section.
    if (var1Size == 0)
    {
        ptr = ptr + Write16(ptr, (Reserved4Bit << 12) | var1Size); 
    }
    else
    {
        WriteHelper<uint16_t> descriptorHelper(ptr, ptr + 2);
        //fill "reserved_future_use + network_descriptors_length" to 0 temporarily.
        ptr = ptr + Write16(ptr, 0);  
        ptr = ptr + descriptors.MakeCodes(ptr, var1Size);
        //rewrite reserved_future_use + network_descriptors_length.
        descriptorHelper.Write(Reserved4Bit << 12, ptr); 
    }

    assert(ptr <= buffer + bufferSize);
    return (ptr - buffer);
}

size_t NitTable::MakeCodes2(uchar_t *buffer, size_t bufferSize,
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
