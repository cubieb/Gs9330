#include "Include/Foundation/SystemInclude.h"
#include <cppunit/extensions/HelperMacros.h>

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/PacketHelper.h"

/* SiTableWrapper */
#include "Include/SiTableWrapper/SiTableXmlWrapperInterface.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Include/TsPacketSiTable/TransportPacketInterface.h"

#include "DescriptorHelper.h"
#include "UtSiTableXmlWrapperInterface.h"
using namespace std;

CxxBeginNameSpace(UnitTest)

/**********************class SiTable**********************/
CPPUNIT_TEST_SUITE_REGISTRATION(SiTableXmlWrapper);

void SiTableXmlWrapper::TestBatXmlWrapperSelect()
{
    TestBatXmlWrapperSelect01();
    TestBatXmlWrapperSelect02();
}

/* private function */
void SiTableXmlWrapper::TestBatXmlWrapperSelect01()
{
    NetId netId = 1;
    BouquetId bouquetId = 1;
    Version   version = 1;
    TsId      tsId = 1;
    OnId      onId = 0;
    uint_t    ccid = 0;
    const size_t BufferSize = 1024;
    size_t    size;

    TsIds tsIds;
    tsIds.push_back(tsId);

    uchar_t bufferL[BufferSize], bufferR[BufferSize]; 
    auto_ptr<TransportPacketInterface> tsPacketR(TransportPacketInterface::CreateInstance(netId, BatPid));
    BatTableInterface *siTableR;
    siTableR = BatTableInterface::CreateInstance(BatTableId, bouquetId, version);
    siTableR->AddTs(tsId, onId);
    tsPacketR->AddSiTable(siTableR);
    size = tsPacketR->GetCodesSize(BatTableId, tsIds);
    tsPacketR->MakeCodes(ccid, BatTableId, tsIds, bufferR, BufferSize);

    char *batXmlString = 
        "<?xml version='1.0'  encoding='gb2312' ?> \n"
        "<Root TableID='0x4A'> \n"
	    "    <Bouquet BouquetID='1' Version='1'> \n"
		"        <Descriptors> \n"
		"        </Descriptors> \n"
        "        <Transportstream TSID='1' ONID='0'> \n"
        "            <Descriptors> \n"
        "            </Descriptors> \n"
        "        </Transportstream> \n"
	    "    </Bouquet> \n"
        "</Root>";
    std::fstream batXml("bat.xml", ios_base::out);
    batXml << batXmlString;
    batXml.close();

    TableId tableId;
    list<SiTableKey> keysL, keysR;    
    keysR.push_back(1);
    auto_ptr<TransportPacketInterface> tsPacketL(TransportPacketInterface::CreateInstance(netId, BatPid));
    BatXmlWrapper<TransportPacketInterface, BatTableInterface> wrapper;
    wrapper.Select(*tsPacketL, "bat.xml", tableId, keysL);
    CPPUNIT_ASSERT(tableId == 0x4A);
    CPPUNIT_ASSERT(keysL == keysR);
    CPPUNIT_ASSERT(tsPacketL->GetCodesSize(BatTableId, tsIds) == size);
    tsPacketL->MakeCodes(ccid, BatTableId, tsIds, bufferL, BufferSize);
    CPPUNIT_ASSERT(memcmp(bufferL, bufferR, size) == 0);
    remove("bat.xml");
}

void SiTableXmlWrapper::TestBatXmlWrapperSelect02()
{
    NetId netId = 1;
    BouquetId bouquetId = 1;
    Version   version = 1;
    TsId      tsId = 1;
    OnId      onId = 0;
    uint_t    ccid = 0;
    const size_t BufferSize = 1024;
    size_t    size;

    TsIds tsIds;
    tsIds.push_back(tsId);

    uchar_t bufferL[BufferSize], bufferR[BufferSize]; 
    auto_ptr<TransportPacketInterface> tsPacketR(TransportPacketInterface::CreateInstance(netId, BatPid));
    BatTableInterface *siTableR;
    siTableR = BatTableInterface::CreateInstance(BatTableId, bouquetId, version);
    siTableR->AddDescriptor(string("FE06000102030405"));
    siTableR->AddTs(tsId, onId);
    siTableR->AddTsDescriptor(tsId, string("FE06000102030405"));
    tsPacketR->AddSiTable(siTableR);
    size = tsPacketR->GetCodesSize(BatTableId, tsIds);
    tsPacketR->MakeCodes(ccid, BatTableId, tsIds, bufferR, BufferSize);

    char *batXmlString = 
        "<?xml version='1.0'  encoding='gb2312' ?> \n"
        "<Root TableID='0x4A'> \n"
        "    <Bouquet BouquetID='1' Version='1'> \n"
        "        <Descriptors> \n"
        "            <Item01 Tag='0xFE' Data='FE06000102030405'/> \n"
        "        </Descriptors> \n"
        "        <Transportstream TSID='1' ONID='0'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0xFE' Data='FE06000102030405'/> \n"
        "            </Descriptors> \n"
        "        </Transportstream> \n"
        "    </Bouquet> \n"
        "</Root> \n";
    std::fstream batXml("bat.xml", ios_base::out);
    batXml << batXmlString;
    batXml.close();

    TableId tableId;
    list<SiTableKey> keysL, keysR;    
    keysR.push_back(1);
    auto_ptr<TransportPacketInterface> tsPacketL(TransportPacketInterface::CreateInstance(netId, BatPid));
    BatXmlWrapper<TransportPacketInterface, BatTableInterface> wrapper;
    wrapper.Select(*tsPacketL, "bat.xml", tableId, keysL);
    CPPUNIT_ASSERT(tableId == 0x4A);
    CPPUNIT_ASSERT(keysL == keysR);
    CPPUNIT_ASSERT(tsPacketL->GetCodesSize(BatTableId, tsIds) == size);
    tsPacketL->MakeCodes(ccid, BatTableId, tsIds, bufferL, BufferSize);
    CPPUNIT_ASSERT(memcmp(bufferL, bufferR, size) == 0);
    remove("bat.xml");
}

CxxEndNameSpace