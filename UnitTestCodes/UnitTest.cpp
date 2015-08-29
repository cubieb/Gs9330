#include "SystemInclude.h"
#include "Common.h"
#include "Crc32.h"

#include "Nit.h"
#include "XmlDataWrapper.h"
#include "Ts.h"
#include "Controller.h"
#include "UnitTest.h"

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

CxxBeginNameSpace(UnitTest)
#if 0

/**********************class XmlDataWrapperTestCase**********************/
CPPUNIT_TEST_SUITE_REGISTRATION( XmlDataWrapperTestCase );
void XmlDataWrapperTestCase::setUp()
{
} 

void XmlDataWrapperTestCase::Trigger(const DataWrapper<Nit>& wrapper) const
{
    /* do nothing */
}

void XmlDataWrapperTestCase::TestTrigger()
{
    CPPUNIT_ASSERT(true);
} 

void XmlDataWrapperTestCase::TestFillNit()
{
    Nit nit1, nit2;

    function<void(const DataWrapper<Nit>&)> trigger(bind(&XmlDataWrapperTestCase::Trigger, this, _1));
    NitXmlWrapper<Nit> nitWrapper(trigger, "../XmlFiles/Nit.UnitTest1.xml"); 
    nitWrapper.Fill(nit1);

    nit2.SetTableId(0x40);
    nit2.SetNetworkId(1);
    nit2.SetVersionNumber(1);
    char *name = "Chengdu Broadcast Television Network";
    nit2.AddDescriptor(0x40, (uchar_t*)name, strlen(name)); 

    uint16_t tsId = 0x1;
    uint16_t onId = 0x65;
    uint32_t freq = 0x04190000;
    uint16_t fecOuter = 0x2;
    uchar_t  modulation = 0x3;
    uint32_t symbolRate = 0x68750;
    uint32_t fecInner = 0x0;

    nit2.AddTs(tsId, onId);
    nit2.AddTsDescriptor0x44(tsId, freq, fecOuter, modulation, symbolRate, fecInner);

    CPPUNIT_ASSERT(nit1 == nit2);
} 

/**********************class NitCase**********************/
CPPUNIT_TEST_SUITE_REGISTRATION(NitCase);
void NitCase::setUp()
{
}

void NitCase::TestMakeCodes()
{
    Nit nit;
    
    nit.SetTableId(0x40);
    nit.SetNetworkId(1);
    nit.SetVersionNumber(0xc);
    char *netName = "Royal cable";
    nit.AddDescriptor(0x40, (uchar_t*)netName, strlen(netName)); 

    uint16_t tsId, onId;
    list<pair<uint16_t, uchar_t>> serviceList;

    /////////////////////////////////
    tsId = 0x1;
    onId = 0x0;
    nit.AddTs(tsId, onId);

    serviceList.clear();
    serviceList.push_back(make_pair(1, 1));
    serviceList.push_back(make_pair(2, 1));
    serviceList.push_back(make_pair(3, 1));
    serviceList.push_back(make_pair(4, 1));
    serviceList.push_back(make_pair(5, 1));
    serviceList.push_back(make_pair(8, 1));
    serviceList.push_back(make_pair(9, 1));    
    serviceList.push_back(make_pair(6, 2));
    serviceList.push_back(make_pair(7, 2));
    nit.AddTsDescriptor0x41(tsId, serviceList);

    nit.AddTsDescriptor0x44(tsId, 0x03060000, 0, 5, 0x68750, 0);
    uchar_t descriptor083TsId1[] = 
    { 
        0x00, 0x01, 0x00, 0x04, 0x00, 0x02, 0x00, 0x07, 0x00, 0x03, 0x00, 0x06, 0x00, 0x04, 0x00, 0x01,
        0x00, 0x05, 0x00, 0x03, 0x00, 0x08, 0x00, 0x02, 0x00, 0x09, 0x00, 0x05, 0x00, 0x06, 0x01, 0xF4,
        0x00, 0x07, 0x01, 0xF5
    };
    nit.AddTsDescriptor(tsId, 0x83, descriptor083TsId1, 36); 

    size_t size = nit.GetCodesSize();
    shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
    size_t ret = nit.MakeCodes(buffer.get(), size);
    CPPUNIT_ASSERT(ret == size);

    uint32_t nitCrc;
    Read32(buffer.get() + size - 4, nitCrc);
    CPPUNIT_ASSERT(nitCrc == 0xa711da84);
} 

///**********************class ControllerCase**********************/
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

CPPUNIT_TEST_SUITE_REGISTRATION(TsCase);
void TsCase::setUp()
{
}

void TsCase::Trigger(const DataWrapper<Nit>& wrapper) const
{
    /* do nothing */
}

void TsCase::TestMakeCodes()
{
    Nit nit;
    
    nit.SetTableId(0x40);
    nit.SetNetworkId(1);
    nit.SetVersionNumber(0xc);
    char *netName = "Royal cable";
    nit.AddDescriptor(0x40, (uchar_t*)netName, strlen(netName)); 

    uint16_t tsId, onId;
    list<pair<uint16_t, uchar_t>> serviceList;

    /////////////////////////////////
    tsId = 0x1;
    onId = 0x0;
    nit.AddTs(tsId, onId);

    serviceList.clear();
    serviceList.push_back(make_pair(1, 1));
    serviceList.push_back(make_pair(2, 1));
    serviceList.push_back(make_pair(3, 1));
    serviceList.push_back(make_pair(4, 1));
    serviceList.push_back(make_pair(5, 1));
    serviceList.push_back(make_pair(8, 1));
    serviceList.push_back(make_pair(9, 1));    
    serviceList.push_back(make_pair(6, 2));
    serviceList.push_back(make_pair(7, 2));
    nit.AddTsDescriptor0x41(tsId, serviceList);

    nit.AddTsDescriptor0x44(tsId, 0x03060000, 0, 5, 0x68750, 0);
    uchar_t descriptor083TsId1[] = 
    { 
        0x00, 0x01, 0x00, 0x04, 0x00, 0x02, 0x00, 0x07, 0x00, 0x03, 0x00, 0x06, 0x00, 0x04, 0x00, 0x01,
        0x00, 0x05, 0x00, 0x03, 0x00, 0x08, 0x00, 0x02, 0x00, 0x09, 0x00, 0x05, 0x00, 0x06, 0x01, 0xF4,
        0x00, 0x07, 0x01, 0xF5
    };
    nit.AddTsDescriptor(tsId, 0x83, descriptor083TsId1, 36); 

    std::cout << nit;

    Ts ts;
    size_t size = ts.GetCodesSize(nit);
    shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
    size_t ret = ts.MakeCodes(nit, buffer.get(), size);
    
    Crc32 crc32;
    uint32_t tsCrc = crc32.CalculateCrc(buffer.get(), size);
    CPPUNIT_ASSERT(tsCrc == 0x77362314);
} 

#endif
CxxEndNameSpace
//EOF