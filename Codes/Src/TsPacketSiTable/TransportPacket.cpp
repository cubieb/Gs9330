#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/PacketHelper.h"
#include "Include/Foundation/Converter.h"
#include "Include/Foundation/Crc32.h"
#include "Include/Foundation/Deleter.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Include/TsPacketSiTable/TransportPacketInterface.h"
#include "TransportPacket.h"
using namespace std;

TransportPacketInterface * TransportPacketInterface::CreateInstance(NetId netId, Pid pid)
{
    if (pid != NitPid && pid != BatPid && pid != SdtPid && pid != EitPid)
        return nullptr;

    return new TransportPacket(netId, pid);
}

TransportPacketsInterface * TransportPacketsInterface::CreateInstance()
{
    return new TransportPackets;
}

/**********************class TransportPacket**********************/
TransportPacket::TransportPacket(NetId netId, Pid pid)
    : adaptationFieldControl(1), pid(pid), netId(netId),
      transportPriority(0)
{}

TransportPacket::~TransportPacket()
{
    for_each(siTables.begin(), siTables.end(), ScalarDeleter());
}

void TransportPacket::AddSiTable(SiTableInterface *siTable)
{
    TableId tableId = siTable->GetTableId();
    switch (pid)
    {
    case BatPid:
        assert(tableId == BatTableId 
               || tableId == SdtActualTableId
               || tableId == SdtOtherTableId);
        break;

    case EitPid:
        assert(tableId == EitActualPfTableId 
               || tableId == EitOtherPfTableId
               || tableId == EitActualSchTableId
               || tableId == EitOtherSchTableId);
        break;
        
    case NitPid:
        assert(tableId == NitActualTableId 
               || tableId == NitOtherTableId);
        break;
    };
    siTables.push_back(siTable);
}

void TransportPacket::DelSiTable(TableId tableId, SiTableKey key)
{
    list<SiTableInterface *>::iterator iter;
    iter = find_if(siTables.begin(), siTables.end(), CompareSiTableIdAndKey(tableId, key));
    if (iter != siTables.end())
    {        
        delete *iter;
        siTables.erase(iter);
    }
}

SiTableInterface * TransportPacket::FindSiTable(TableId tableId, SiTableKey key)
{
    list<SiTableInterface *>::iterator iter;
    iter = find_if(siTables.begin(), siTables.end(), CompareSiTableIdAndKey(tableId, key));
    return iter == siTables.end()? nullptr: *iter;
}

size_t TransportPacket::GetCodesSize(TableId tableId, TsId tsId) const
{
    uint_t packetNumber = 0; 

    for (auto iter: siTables)
    {
        uint_t secNumber = iter->GetSecNumber(tableId, tsId);
        for (uint_t i = 0; i < secNumber; ++i)
        {            
            size_t tableSize = iter->GetCodesSize(tableId, tsId, i);
            assert(tableSize != 0);

            //+1 for pointer_field
            packetNumber = packetNumber + GetPacketNumber(tableSize + 1); 
        }        
    }

    return (TsPacketSize * packetNumber);
}

NetId TransportPacket::GetNetId() const
{
    return netId;
}

Pid TransportPacket::GetPid() const
{
    return pid;
}

uint_t TransportPacket::GetPacketNumber(size_t codesSize) const
{
    assert(codesSize != 0);
    return (codesSize + MaxTsPacketPayloadSize - 1) / MaxTsPacketPayloadSize;
}

size_t TransportPacket::MakeCodes(CcId ccId, TableId tableId, TsId tsId, 
                                  uchar_t *buffer, size_t bufferSize)
{
    uchar_t *ptr = buffer;
    assert(GetCodesSize(tableId, tsId) <= bufferSize);

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
        SectionNumber secNumber = (SectionNumber)iter->GetSecNumber(tableId, tsId);
        for (SectionNumber i = 0; i < secNumber; ++i)
        { 
            size_t tablePlainSize = iter->GetCodesSize(tableId, tsId, i);
            assert(tablePlainSize != 0);

            //+1 for pointer_field
            uint_t packetNumber = GetPacketNumber(tablePlainSize + 1); 
            //pointer_field and 0xff tail included
            size_t tableExtSize = MaxTsPacketPayloadSize * packetNumber;  

            shared_ptr<uchar_t> tableCodes(new uchar_t[tableExtSize], ArrayDeleter());
            Write8(tableCodes.get(), 0x0); //pointer_field
            iter->MakeCodes(tableId, tsId, tableCodes.get() + 1, tablePlainSize, i);
            memset(tableCodes.get() + 1 + tablePlainSize, 0xff, tableExtSize - 1 - tablePlainSize);

            for (uint_t i = 0; i < packetNumber; ++i)
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
                /* The continuity_counter is a 4-bit field incrementing with each 
                   Transport Stream packet with the same PID.
                 */
                ptr = ptr + Write8(ptr, (adaptationFieldControl << 4) | (ccIter->second++ & 0xF)); 
                ptr = ptr + Write(ptr, MaxTsPacketPayloadSize, 
                                  tableCodes.get() + MaxTsPacketPayloadSize * i, MaxTsPacketPayloadSize);
            } //for (uint_t i = 0; i < packetNumber; ++i)
        } //for (uint_t i = 0; i < secNumber; ++i)
    } //for (auto iter: siTables)
        
    return ptr - buffer;
}

void TransportPacket::RefreshCatch()
{
    list<SiTableInterface *>::iterator iter;
    for (iter = siTables.begin(); iter != siTables.end(); ++iter)
    {
        (*iter)->RefreshCatch();
    }
}

/**********************class TransportPackets**********************/
TransportPackets::TransportPackets()
{
    AllocProxy();
}

TransportPackets::~TransportPackets()
{
    FreeProxy();
    for_each(tsPackets.begin(), tsPackets.end(), ScalarDeleter());
}

void TransportPackets::Add(TransportPacketInterface *tsPacket)
{
    tsPackets.push_back(tsPacket);
}

TransportPackets::iterator TransportPackets::Begin()
{
    return iterator(this, NodePtr(tsPackets.begin()));
}

void TransportPackets::Delete(NetId netId, Pid pid)
{
    list<TransportPacketInterface *>::iterator iter;
    iter = find_if(tsPackets.begin(), tsPackets.end(), CompareTsPacketNetIdAndPid(netId, pid));
    if (iter != tsPackets.end())
    {
        tsPackets.erase(iter);
        delete *iter;
    }
}

TransportPackets::iterator TransportPackets::End()
{
    return iterator(this, NodePtr(tsPackets.end()));
}

TransportPackets::iterator TransportPackets::Find(NetId netId, Pid pid)
{
    list<TransportPacketInterface *>::iterator iter;
    iter = find_if(tsPackets.begin(), tsPackets.end(), CompareTsPacketNetIdAndPid(netId, pid));
    return iterator(this, iter);
}

TransportPackets::NodePtr TransportPackets::GetMyHead()
{
    return NodePtr(tsPackets.end());
}
