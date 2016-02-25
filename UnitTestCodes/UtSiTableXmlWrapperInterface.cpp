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

void SiTableXmlWrapper::TestEitXmlWrapperSelect()
{
    char *xmlString = 
        "<?xml version='1.0'  encoding='gb2312' ?> \n"
        "<Root TableID='0x4E'> \n"
        "    <Transportstream TSID='1' ONID='201' Version='1' ServiceID='1'> \n"
        "        <Event EventID='288' StartTime='2016-01-14 09:00:00' Duration='003600' running_status='4' free_CA_mode='1'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0x4D' Data='4D13zho07爱丽丝梦游仙境07爱丽丝和小兔子'/> \n"
        "            </Descriptors> \n"
        "        </Event> \n"
        "        <Event EventID='287' StartTime='2016-01-14 10:00:00' Duration='003600' running_status='4' free_CA_mode='1'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0x4D' Data='4D17zho05七个小矮人0D白雪公主和七个小矮人的故事'/> \n"
        "            </Descriptors> \n"
        "        </Event> \n"
        "    </Transportstream> \n"
        "    <Transportstream TSID='1' ONID='201' Version='1' ServiceID='4'> \n"
        "        <Event EventID='16' StartTime='2016-01-14 09:00:00' Duration='003600' running_status='4' free_CA_mode='1'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0x4A' Data='4A07000100C8001401'/> \n"
        "                <Item02 Tag='0x4D' Data='4D13zho07爱丽丝梦游仙境07爱丽丝和小兔子'/> \n"
        "                <Item03 Tag='0x4F' Data='4F0400000000'/> \n"
        "            </Descriptors> \n"
        "        </Event> \n"
        "        <Event EventID='15' StartTime='2016-01-14 10:00:00' Duration='003600' running_status='4' free_CA_mode='1'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0x4D' Data='4D17zho05七个小矮人0D白雪公主和七个小矮人的故事'/> \n"
        "            </Descriptors> \n"
        "        </Event> \n"
        "        <Event EventID='14' StartTime='2016-01-14 11:00:00' Duration='003600' running_status='4' free_CA_mode='1'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0x4D' Data='4D0Ezho03美人鱼06美人鱼和王子'/> \n"
        "            </Descriptors> \n"
        "        </Event> \n"
        "    </Transportstream> \n"
        "</Root>";
}

void SiTableXmlWrapper::TestNitXmlWrapperSelect()
{
    char *xmlString = 
        "<?xml version='1.0'  encoding='gb2312' ?> \n"
        "<Root TableID='0x40'> \n"
        "    <Network ID='201' Version='12'> \n"
        "        <Descriptors> \n"
        "            <Item01 Tag='0x40' Data='4003金牛区'/> \n"
        "        </Descriptors> \n"
        "        <Transportstream TSID='1' ONID='201'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0x44' Data='440B04740000FFF00300687500'/> \n"
        "                <Item02 Tag='0x41' Data='410C000101000201000301000401'/> \n"
        "            </Descriptors> \n"
        "        </Transportstream> \n"
        "        <Transportstream TSID='2' ONID='201'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0x44' Data='440B04820000FFF00300687500'/> \n"
        "                <Item02 Tag='0x41' Data='410C000501000601000701000801'/> \n"
        "            </Descriptors> \n"
        "        </Transportstream> \n"
        "        <Transportstream TSID='3' ONID='201'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0x44' Data='440B04900000FFF00300687500'/> \n"
        "                <Item02 Tag='0x41' Data='410C000901000a01000b01000c01'/> \n"
        "            </Descriptors> \n"
        "        </Transportstream> \n"
        "        <Transportstream TSID='4' ONID='201'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0x44' Data='440B04980000FFF00300687500'/> \n"
        "                <Item02 Tag='0x41' Data='410C000e01000f01001001001101'/> \n"
        "            </Descriptors> \n"
        "        </Transportstream> \n"
        "    </Network> \n"
        "</Root>";
}

void SiTableXmlWrapper::TestSdtXmlWrapperSelect()
{
    char *xmlString = 
        "<?xml version='1.0'  encoding='gb2312' ?> \n"
        "<Root TableType='SDT' TableID='0x42'> \n"
        "    <Transportstream TSID='1' ONID='201' Version='0'> \n"
        "        <Service ServiceID='1' EIT_schedule_flag='1' EIT_present_following_flag='1' running_status='4' free_CA_mode='0'> \n"
        "            <Descriptors Sum='1'> \n"
        "                <Item1 Tag='0x48' Data='480A010005CCTV01'/> \n"
        "            </Descriptors> \n"
        "        </Service> \n"
        "        <Service ServiceID='2' EIT_schedule_flag='1' EIT_present_following_flag='1' running_status='4' free_CA_mode='0'> \n"
        "            <Descriptors Sum='1'> \n"
        "                <Item1 Tag='0x48' Data='480A010006CCTV02'/> \n"
        "            </Descriptors> \n"
        "        </Service> \n"
        "        <Service ServiceID='3' EIT_schedule_flag='1' EIT_present_following_flag='1' running_status='4' free_CA_mode='0'> \n"
        "            <Descriptors Sum='1'> \n"
        "                <Item1 Tag='0x48' Data='480A010006CCTV03'/> \n"
        "            </Descriptors> \n"
        "        </Service> \n"
        "        <Service ServiceID='4' EIT_schedule_flag='1' EIT_present_following_flag='1' running_status='4' free_CA_mode='0'> \n"
        "            <Descriptors Sum='1'> \n"
        "                <Item1 Tag='0x48' Data='480C010006CCTV04'/> \n"
        "            </Descriptors> \n"
        "        </Service> \n"
        "    </Transportstream> \n"
        "</Root>";
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
    
    uchar_t bufferL[BufferSize], bufferR[BufferSize]; 
    auto_ptr<TransportPacketInterface> tsPacketR(TransportPacketInterface::CreateInstance(netId, BatPid));
    SiTableInterface *siTableR;
    siTableR = SiTableInterface::CreateBatInstance(BatTableId, bouquetId, version);
    siTableR->AddTs(tsId, onId);
    tsPacketR->AddSiTable(siTableR);
    size = tsPacketR->GetCodesSize(BatTableId, tsId);
    tsPacketR->MakeCodes(ccid, BatTableId, tsId, bufferR, BufferSize);

    char *xmlString = 
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
    batXml << xmlString;
    batXml.close();

    TableId tableId;
    list<SiTableKey> keysL, keysR;    
    keysR.push_back(1);
    auto_ptr<TransportPacketInterface> tsPacketL(TransportPacketInterface::CreateInstance(netId, BatPid));
    BatXmlWrapper<TransportPacketInterface, SiTableInterface> wrapper;
    wrapper.Select(*tsPacketL, "bat.xml", tableId, keysL);
    CPPUNIT_ASSERT(tableId == 0x4A);
    CPPUNIT_ASSERT(keysL == keysR);
    CPPUNIT_ASSERT(tsPacketL->GetCodesSize(BatTableId, tsId) == size);
    tsPacketL->MakeCodes(ccid, BatTableId, tsId, bufferL, BufferSize);
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

    uchar_t bufferL[BufferSize], bufferR[BufferSize]; 
    auto_ptr<TransportPacketInterface> tsPacketR(TransportPacketInterface::CreateInstance(netId, BatPid));
    SiTableInterface *siTableR;
    siTableR = SiTableInterface::CreateBatInstance(BatTableId, bouquetId, version);
    siTableR->AddDescriptor(string("4704Yule"));
    siTableR->AddTs(tsId, onId);
    siTableR->AddTsDescriptor(tsId, string("410F001501001401001301001601001201"));
    tsPacketR->AddSiTable(siTableR);
    size = tsPacketR->GetCodesSize(BatTableId, tsId);
    tsPacketR->MakeCodes(ccid, BatTableId, tsId, bufferR, BufferSize);

    char *xmlString = 
        "<?xml version='1.0'  encoding='gb2312' ?> \n"
        "<Root TableID='0x4A'> \n"
        "    <Bouquet BouquetID='1' Version='1'> \n"
        "        <Descriptors> \n"
        "            <Item01 Tag='0x47' Data='4704Yule'/> \n"
        "        </Descriptors> \n"
        "        <Transportstream TSID='1' ONID='0'> \n"
        "            <Descriptors> \n"
        "                <Item01 Tag='0x41' Data='410F001501001401001301001601001201'/> \n"
        "            </Descriptors> \n"
        "        </Transportstream> \n"
        "    </Bouquet> \n"
        "</Root> \n";
    std::fstream batXml("bat.xml", ios_base::out);
    batXml << xmlString;
    batXml.close();

    TableId tableId;
    list<SiTableKey> keysL, keysR;    
    keysR.push_back(1);
    auto_ptr<TransportPacketInterface> tsPacketL(TransportPacketInterface::CreateInstance(netId, BatPid));
    BatXmlWrapper<TransportPacketInterface, SiTableInterface> wrapper;
    wrapper.Select(*tsPacketL, "bat.xml", tableId, keysL);
    CPPUNIT_ASSERT(tableId == 0x4A);
    CPPUNIT_ASSERT(keysL == keysR);
    CPPUNIT_ASSERT(tsPacketL->GetCodesSize(BatTableId, tsId) == size);
    tsPacketL->MakeCodes(ccid, BatTableId, tsId, bufferL, BufferSize);
    CPPUNIT_ASSERT(memcmp(bufferL, bufferR, size) == 0);
    remove("bat.xml");
}

CxxEndNameSpace