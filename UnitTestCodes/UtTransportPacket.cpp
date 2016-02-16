#include "Include/Foundation/SystemInclude.h"
#include <cppunit/extensions/HelperMacros.h>

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/PacketHelper.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Include/TsPacketSiTable/TransportPacketInterface.h"

#include "DescriptorHelper.h"
#include "UtSiTable.h"
#include "UtTransportPacket.h"
using namespace std;

CxxBeginNameSpace(UnitTest)

/**********************class TransportPacket**********************/
CPPUNIT_TEST_SUITE_REGISTRATION(TransportPacket);
void TransportPacket::TestTransportPacketConstruct()
{
    NetId netId = 1;
    TransportPacketInterface *tsPacket;

    tsPacket = TransportPacketInterface::CreateInstance(netId, InvalidPid);
    CPPUNIT_ASSERT(tsPacket == nullptr);

    tsPacket = TransportPacketInterface::CreateInstance(netId, BatPid);
    CPPUNIT_ASSERT(tsPacket != nullptr);
    delete tsPacket;
}

void TransportPacket::TestTransportPacketAddSiTable()
{
    NetId netId = 1;
    BouquetId bouquetId = 1;
    Version   version = 1;

    auto_ptr<TransportPacketInterface> tsPacket(TransportPacketInterface::CreateInstance(netId, BatPid));

    SiTableInterface *siTable;
    siTable = BatTableInterface::CreateInstance(BatTableId, bouquetId, version);
    tsPacket->AddSiTable(siTable);
    CPPUNIT_ASSERT(tsPacket->FindSiTable(BatTableId, bouquetId) != nullptr);
}

void TransportPacket::TestTransportPacketDelSiTable()
{
    NetId netId = 1;
    BouquetId bouquetId1 = 1;
    BouquetId bouquetId2 = 2;
    Version   version = 1;

    auto_ptr<TransportPacketInterface> tsPacket(TransportPacketInterface::CreateInstance(netId, BatPid));

    SiTableInterface *siTable;
    siTable = BatTableInterface::CreateInstance(BatTableId, bouquetId1, version);
    tsPacket->AddSiTable(siTable);    
    siTable = BatTableInterface::CreateInstance(BatTableId, bouquetId2, version);
    tsPacket->AddSiTable(siTable);

    tsPacket->DelSiTable(BatTableId, bouquetId1);
    CPPUNIT_ASSERT(tsPacket->FindSiTable(BatTableId, bouquetId1) == nullptr);
    CPPUNIT_ASSERT(tsPacket->FindSiTable(BatTableId, bouquetId2) != nullptr);

    tsPacket->DelSiTable(BatTableId, bouquetId2);
    CPPUNIT_ASSERT(tsPacket->FindSiTable(BatTableId, bouquetId1) == nullptr);
    CPPUNIT_ASSERT(tsPacket->FindSiTable(BatTableId, bouquetId2) == nullptr);
}

void TransportPacket::TestTransportPacketFindSiTable()
{
    TestTransportPacketAddSiTable();
    TestTransportPacketDelSiTable();
}

void TransportPacket::TestTransportPacketGetCodesSize()
{
    NetId netId = 1;
    BouquetId bouquetId = 1;
    Version   version = 1;
    TsId      tsId = 1;
    OnId      onId = 0;

    TsIds tsIds;
    tsIds.push_back(tsId);

    auto_ptr<TransportPacketInterface> tsPacket(TransportPacketInterface::CreateInstance(netId, BatPid));
    CPPUNIT_ASSERT(tsPacket->GetCodesSize(BatTableId,tsIds) == 0);

    BatTableInterface *siTable;
    siTable = BatTableInterface::CreateInstance(BatTableId, bouquetId, version);
    tsPacket->AddSiTable(siTable);
    CPPUNIT_ASSERT(tsPacket->GetCodesSize(BatTableId,tsIds) == TsPacketSize);

    //-1 for pointer_field
    size_t size = MaxTsPacketPayloadSize - 1 - sizeof(bouquet_association_section);
    siTable->AddDescriptor(GetDescriptorString(size));
    CPPUNIT_ASSERT(tsPacket->GetCodesSize(BatTableId,tsIds) == TsPacketSize);

    siTable->AddDescriptor(GetDescriptorString(2));
    CPPUNIT_ASSERT(tsPacket->GetCodesSize(BatTableId,tsIds) == TsPacketSize * 2);
}

void TransportPacket::TestTransportPacketGetNetId()
{
    NetId     netId = 1;
    BouquetId bouquetId = 2;
    Version   version = 3;
    
    auto_ptr<TransportPacketInterface> tsPacket(TransportPacketInterface::CreateInstance(netId, BatPid));
    CPPUNIT_ASSERT(tsPacket->GetNetId() == netId);
}

void TransportPacket::TestTransportPacketGetPid()
{
    NetId     netId = 1;
    BouquetId bouquetId = 2;
    Version   version = 3;
    
    auto_ptr<TransportPacketInterface> tsPacket(TransportPacketInterface::CreateInstance(netId, BatPid));
    CPPUNIT_ASSERT(tsPacket->GetPid() == BatPid);
}

/* SiTable with only one section */
void TransportPacket::TestTransportPacketMakeCodes1()
{
    NetId     netId = 1;
    BouquetId bouquetId = 2;
    Version   version = 3;
    TsId      tsId = 1;
    OnId      onId = 0;

    TsIds tsIds;
    tsIds.push_back(tsId);
    
    auto_ptr<TransportPacketInterface> tsPacket(TransportPacketInterface::CreateInstance(netId, BatPid));
    static uchar_t buffer[2048];

    CPPUNIT_ASSERT(tsPacket->MakeCodes(0, BatTableId, tsIds, buffer, 2048) == 0);

    BatTableInterface *siTable;
    siTable = BatTableInterface::CreateInstance(BatTableId, bouquetId, version);
    tsPacket->AddSiTable(siTable);

    CPPUNIT_ASSERT(tsPacket->MakeCodes(0, BatTableId, tsIds, buffer, 2048) == TsPacketSize);
    uchar_t code1[] = { 0x47, 0x40, 0x11, 0x10 };
    CPPUNIT_ASSERT(memcmp(buffer, code1, 4) == 0);

    CPPUNIT_ASSERT(tsPacket->MakeCodes(0, BatTableId, tsIds, buffer, 2048) == TsPacketSize);
    uchar_t code2[] = { 0x47, 0x40, 0x11, 0x11 };
    CPPUNIT_ASSERT(memcmp(buffer, code2, 4) == 0);

    //-1 for pointer_field
    size_t size = MaxTsPacketPayloadSize - 1 - sizeof(bouquet_association_section);
    siTable->AddDescriptor(GetDescriptorString(size));
    siTable->AddDescriptor(GetDescriptorString(2));
    CPPUNIT_ASSERT(tsPacket->MakeCodes(0, BatTableId, tsIds, buffer, 2048) == TsPacketSize * 2);
    uchar_t code3[] = { 0x47, 0x40, 0x11, 0x12 };
    uchar_t code4[] = { 0x47, 0x00, 0x11, 0x13 };
    CPPUNIT_ASSERT(memcmp(buffer, code3, 4) == 0);
    CPPUNIT_ASSERT(memcmp(buffer + TsPacketSize, code4, 4) == 0);
}

/* SiTable with two or more sections, and performance test!!! */
void TransportPacket::TestTransportPacketMakeCodes2()
{
    NetId     netId = 1;
    BouquetId bouquetId = 2;
    Version   version = 3;
    
    auto_ptr<TransportPacketInterface> tsPacket(TransportPacketInterface::CreateInstance(netId, BatPid));
    #define MaxBufferSize (1024 * 16)
    static uchar_t buffer[MaxBufferSize];

    uchar_t code[] = { 0x47, 0x40, 0x11, 0x10, 0x00 };
    BatTableInterface *siTable;
    siTable = BatTableInterface::CreateInstance(BatTableId, bouquetId, version);
    tsPacket->AddSiTable(siTable);

    uint_t tsNumber = 200;  //1300
    uint_t sectionUnitTsNumber = MaxBatDesAndTsContentSize / sizeof(transport_stream);
    uint_t headSectionNumber = tsNumber / sectionUnitTsNumber;
    uint_t tailSectionNumber = (tsNumber % sectionUnitTsNumber) == 0 ? 0 : 1;
    size_t headSectionUnitSize = sizeof(transport_stream) * sectionUnitTsNumber
                             + sizeof(bouquet_association_section)
                             + 1;  //+1 for pointer_field
    size_t tailSectionSize = (tsNumber - sectionUnitTsNumber * headSectionNumber) * sizeof(transport_stream)
                             + sizeof(bouquet_association_section);
    uint_t headSectionUnitPacketNumber = (headSectionUnitSize + TsPacketSize - 1) / TsPacketSize;
    uint_t tailSectionUnitPacketNumber = (tailSectionSize + TsPacketSize - 1) / TsPacketSize;
    size_t tsPacketNumber = headSectionUnitPacketNumber * headSectionNumber + tailSectionUnitPacketNumber;
    
    OnId      onId = 0;
    TsIds tsIds;

    uint_t i;
    for (i = 0; i < tsNumber; ++i)
    {
        TsId tsId = i + 1;
        tsIds.push_back(tsId);
        siTable->AddTs(tsId, onId);
    }

    size_t codeSize = tsPacket->GetCodesSize(BatTableId, tsIds);
    CPPUNIT_ASSERT(codeSize == TsPacketSize * tsPacketNumber);
    CPPUNIT_ASSERT(codeSize == tsPacket->MakeCodes(0, BatTableId, tsIds, buffer, MaxBufferSize));
        
    for (i = 1; i < tsPacketNumber; ++i)
    {
        size_t cmpSize;
        if (i % headSectionUnitPacketNumber == 0)
        {
            cmpSize = 5;
            code[1] = 0x40;
        }
        else
        {
            cmpSize = 4;
            code[1] = 0x00;
        }
        code[3] = 0x10 | (0x0F & i);
        CPPUNIT_ASSERT(memcmp(buffer + 188 * i, code, cmpSize) == 0);
    }
}

/**********************class TransportPackets**********************/
CPPUNIT_TEST_SUITE_REGISTRATION(TransportPackets);
void TransportPackets::TestTransportPacketsBegin()
{
    auto_ptr<TransportPacketsInterface> tsPackets(TransportPacketsInterface::CreateInstance());
    CPPUNIT_ASSERT(tsPackets->Begin() == tsPackets->End());

    NetId     netId = 1;
    BouquetId bouquetId = 2;
    Version   version = 3;
    
    TransportPacketInterface *tsPacket = TransportPacketInterface::CreateInstance(netId, BatPid);
    tsPackets->Add(tsPacket);
    CPPUNIT_ASSERT(tsPackets->Begin() != tsPackets->End());
    uint_t count = 0;
    TransportPacketsInterface::iterator iter;
    for (iter = tsPackets->Begin(); iter != tsPackets->End(); ++iter)
    {
        count++;
    }
    CPPUNIT_ASSERT(count == 1);

    iter = tsPackets->Find(netId, BatPid);
    CPPUNIT_ASSERT(iter != tsPackets->End());

    iter = tsPackets->Find(netId + 1, BatPid);
    CPPUNIT_ASSERT(iter == tsPackets->End());

    iter = tsPackets->Find(netId, InvalidPid);
    CPPUNIT_ASSERT(iter == tsPackets->End());
}

CxxEndNameSpace