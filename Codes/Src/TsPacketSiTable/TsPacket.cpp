#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/PacketHelper.h"
#include "Include/Foundation/Converter.h"
#include "Include/Foundation/Crc32.h"
#include "Include/Foundation/Deleter.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Include/TsPacketSiTable/TsPacketInterface.h"
#include "TsPacket.h"
using namespace std;

TsPacketInterface * CreateTsPacketInterface(NetId netId, Pid pid)
{
    return new TsPacket(netId, pid);
}

TsPacketsInterface * CreateTsPacketsInterface()
{
    return new TsPackets;
}

/**********************class TsPacket**********************/
TsPacket::TsPacket(NetId netId, Pid pid)
    : adaptationFieldControl(1), pid(pid), netId(netId),
      transportPriority(0)
{}

TsPacket::~TsPacket()
{
    for_each(siTables.begin(), siTables.end(), ScalarDeleter());
}

void TsPacket::AddSiTable(SiTableInterface *siTable)
{
    siTables.push_back(siTable);
}

void TsPacket::DelSiTable(TableId tableId, uint16_t key)
{
    list<SiTableInterface *>::iterator iter;
    iter = find_if(siTables.begin(), siTables.end(), CompareSiTableIdAndKey(tableId, key));
    if (iter != siTables.end())
    {        
        delete *iter;
        siTables.erase(iter);
    }
}

SiTableInterface * TsPacket::FindSiTable(TableId tableId, uint16_t key)
{
    list<SiTableInterface *>::iterator iter;
    iter = find_if(siTables.begin(), siTables.end(), CompareSiTableIdAndKey(tableId, key));
    return iter == siTables.end()? nullptr: *iter;
}

size_t TsPacket::GetCodesSize(TableId tableId, const std::list<TsId>& tsIds) const
{
    uint_t segmentNumber = 0; 

    for (auto iter: siTables)
    {
        size_t tableSize = iter->GetCodesSize(tableId, tsIds);
        if (tableSize == 0)
        {
            continue;
        }

        segmentNumber = segmentNumber + GetSegmentNumber(tableSize + 1); //+1 for pointer_field
    }

    return (TsPacketSize * segmentNumber);
}

NetId TsPacket::GetNetId() const
{
    return netId;
}

Pid TsPacket::GetPid() const
{
    return pid;
}

uint_t TsPacket::GetSegmentNumber(size_t codesSize) const
{
    assert(codesSize != 0);
    size_t segmentPayloadSize = TsPacketSize - sizeof(transport_packet);
    return (codesSize + segmentPayloadSize - 1) / segmentPayloadSize;
}

size_t TsPacket::MakeCodes(uint_t ccId, TableId tableId, std::list<TsId>& tsIds, 
                           uchar_t *buffer, size_t bufferSize)
{
    size_t segmentPayloadSize = TsPacketSize - sizeof(transport_packet);
    uchar_t *ptr = buffer;

    map<uint_t, uchar_t>::iterator ccIter = continuityCounters.find(ccId);
    if (ccIter == continuityCounters.end())
    {
        pair<map<uint_t, uchar_t>::iterator, bool> pr;
        pr = continuityCounters.insert(make_pair(ccId, 0));
        assert(pr.second == true);
        ccIter = pr.first;
    }

    for (auto iter: siTables)
    {
        size_t tablePlainSize = iter->GetCodesSize(tableId, tsIds);
        if (tablePlainSize == 0)
        {
            continue;
        }

        uint_t segmentNumber = GetSegmentNumber(tablePlainSize + 1); //+1 for pointer_field
        size_t tableExtSize = segmentPayloadSize * segmentNumber;  //pointer_field and 0xff tail included

        shared_ptr<uchar_t> tableCodes(new uchar_t[tableExtSize], ArrayDeleter());
        Write8(tableCodes.get(), 0x0); //pointer_field
        iter->MakeCodes(tableId, tsIds, tableCodes.get() + 1, tablePlainSize);
        memset(tableCodes.get() + 1 + tablePlainSize, 0xff, tableExtSize - 1 - tablePlainSize);

        for (uint_t i = 0; i < segmentNumber; ++i)
        {
            ptr = ptr + Write8(ptr, 0x47);
            /* transport_error_indicator(1 bit), payload_unit_start_indicator(1 bit), 
                transport_priority(1 bit), PID(13 bits)
                transport_error_indicator = 0;
                transport_priority = 0;
                payload_unit_start_indicator = (first byte of current section) ? 1 : 0;
            */
            uint16_t startIndicator = (i == 0 ? 1 : 0);
            ptr = ptr + Write16(ptr, (startIndicator << 14) | (transportPriority << 13) | pid);
            /* transport_scrambling_control[2] = '00';
		        adaptation_field_control[2] = '01';
		        continuity_counter[4] = 'xxxx';
		    */
            /* refer to "2.4.3.3 Semantic definition of fields in Transport Stream packet layer",
                continuity_counter should be increase by 1 in all case.  
                when send udp packet, we may send duplicate packet two time, in this circumstance, the
                continuity_counter keep same with the oringinal packet.
                for example, the udp sending function may like this:
                ts.MakeCodes(buffer, bufferSize);
                for (ptr = buffer; ptr = ptr < buffer + buffersize; buffer + 188)
                {
                    SendUdp(ptr, 188);
                    SendUdp(ptr, 188);   //again
                }
            */
            //The continuity_counter is a 4-bit field incrementing with each Transport Stream packet with the same PID.
            ptr = ptr + Write8(ptr, adaptationFieldControl << 4 | (ccIter->second++ & 0xF)); 
            ptr = ptr + Write(ptr, segmentPayloadSize, tableCodes.get() + segmentPayloadSize * i, segmentPayloadSize);
        }
    }
    return ptr - buffer;
}

/**********************class TsPackets**********************/
TsPackets::TsPackets()
{
    AllocProxy();
}

TsPackets::~TsPackets()
{
    FreeProxy();
    for_each(tsPackets.begin(), tsPackets.end(), ScalarDeleter());
}

void TsPackets::Add(TsPacketInterface *tsPacket)
{
    tsPackets.push_back(tsPacket);
}

TsPackets::iterator TsPackets::Begin()
{
    return iterator(this, NodePtr(tsPackets.begin()));
}

void TsPackets::Delete(NetId netId, Pid pid)
{
    list<TsPacketInterface *>::iterator iter;
    iter = find_if(tsPackets.begin(), tsPackets.end(), CompareTsPacketNetIdAndPid(netId, pid));
    if (iter != tsPackets.end())
    {
        tsPackets.erase(iter);
        delete *iter;
    }
}

TsPackets::iterator TsPackets::End()
{
    return iterator(this, NodePtr(tsPackets.end()));
}

TsPackets::iterator TsPackets::Find(NetId netId, Pid pid)
{
    list<TsPacketInterface *>::iterator iter;
    iter = find_if(tsPackets.begin(), tsPackets.end(), CompareTsPacketNetIdAndPid(netId, pid));
    return iterator(this, iter);
}

TsPackets::NodePtr TsPackets::GetMyHead()
{
    return NodePtr(tsPackets.end());
}
