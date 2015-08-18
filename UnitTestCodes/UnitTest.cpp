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

/**********************class XmlDataWrapperTestCase**********************/
CPPUNIT_TEST_SUITE_REGISTRATION( XmlDataWrapperTestCase );
void XmlDataWrapperTestCase::setUp()
{
} 

void XmlDataWrapperTestCase::Trigger(const NitWrapper<Nit>& wrapper) const
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

    function<void(const NitWrapper<Nit>&)> trigger(bind(&XmlDataWrapperTestCase::Trigger, this, _1));
    NitXmlWrapper<Nit> nitWrapper(trigger, "../XmlFiles/Nit.UnitTest1.xml"); 
    nitWrapper.FillNit(nit1);

    nit2.SetTableId(0x40);
    nit2.SetNetworkId(1);
    nit2.SetVersionNumber(1);
    char *name = "Chengdu Broadcast Television Network";
    nit2.AddDescriptor(0x40, (uchar_t*)name, strlen(name)); 
    nit2.AddTs(5, 5);

    CPPUNIT_ASSERT(nit1 == nit2);
} 

/**********************class CrcTestCase**********************/
CPPUNIT_TEST_SUITE_REGISTRATION(CrcTestCase);
void CrcTestCase::setUp()
{
}

void CrcTestCase::TestCrc32()
{
    Nit nit;
    
    nit.SetTableId(0x40);
    nit.SetNetworkId(1);
    nit.SetVersionNumber(0xc);
    char *name = "Royal cable";
    nit.AddDescriptor(0x40, (uchar_t*)name, strlen(name)); 
    uint16_t tsId, onId;

    shared_ptr<uchar_t> buffer;
    uchar_t *ptr;
    size_t bufferSize;
    
    /////////////////////////////////
    tsId = 0x1;
    onId = 0x0;
    nit.AddTs(tsId, onId);

    list<pair<uint16_t, uchar_t>> serviceList;
    serviceList.push_back(make_pair(1, 1));
    serviceList.push_back(make_pair(2, 1));
    serviceList.push_back(make_pair(3, 1));
    serviceList.push_back(make_pair(4, 1));
    serviceList.push_back(make_pair(5, 1));
    serviceList.push_back(make_pair(8, 1));
    serviceList.push_back(make_pair(9, 1));    
    serviceList.push_back(make_pair(6, 2));
    serviceList.push_back(make_pair(7, 2));
    ServiceListDescriptor serviceListDes1;
    serviceListDes1.SetData(serviceList);
    bufferSize = serviceListDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    serviceListDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x41, buffer.get() + 2, bufferSize - 2); 

    CableDeliverySystemDescriptor cdsDes1;
    cdsDes1.SetData(0x03060000, 0, 5, 0x68750, 0);
    bufferSize = cdsDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    cdsDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x44, buffer.get() + 2, bufferSize - 2); 

    uchar_t descriptor0831[] = 
    { 
        0x00, 0x01, 0x00, 0x04, 0x00, 0x02, 0x00, 0x07, 0x00, 0x03, 0x00, 0x06, 0x00, 0x04, 0x00, 0x01,
        0x00, 0x05, 0x00, 0x03, 0x00, 0x08, 0x00, 0x02, 0x00, 0x09, 0x00, 0x05, 0x00, 0x06, 0x01, 0xF4,
        0x00, 0x07, 0x01, 0xF5
    };
    nit.AddTsDescriptor(tsId, onId, 0x83, descriptor0831, 36); 

    ///////////////////////////////////////
    tsId = 0x2;
    onId = 0x0;
    nit.AddTs(tsId, onId);

    serviceList.clear();
    serviceList.push_back(make_pair(0xc9, 1));
    serviceList.push_back(make_pair(0xca, 1));
    serviceList.push_back(make_pair(0xcb, 1));
    serviceList.push_back(make_pair(0xcc, 1));
    serviceList.push_back(make_pair(0xcd, 1));
    serviceList.push_back(make_pair(0xce, 1));
    serviceList.push_back(make_pair(0xcf, 1));    
    serviceListDes1.SetData(serviceList);
    bufferSize = serviceListDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    serviceListDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x41, buffer.get() + 2, bufferSize - 2); 

    cdsDes1.SetData(0x03140000, 0, 5, 0x68750, 0);
    bufferSize = cdsDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    cdsDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x44, buffer.get() + 2, bufferSize - 2); 

    uchar_t descriptor0832[] = 
    { 
        0x00, 0xC9, 0x00, 0x08, 0x00, 0xCA, 0x00, 0x09, 0x00, 0xCB, 0x00, 0x0A, 0x00, 0xCC, 0x00, 0x0B,
        0x00, 0xCD, 0x00, 0x0C, 0x00, 0xCE, 0x00, 0x0D, 0x00, 0xCF, 0x00, 0x0E
    };
    nit.AddTsDescriptor(tsId, onId, 0x83, descriptor0832, 28); 

    ///////////////////////////////////////
    tsId = 0x3;
    onId = 0x0;
    nit.AddTs(tsId, onId);

    serviceList.clear();
    serviceList.push_back(make_pair(0x012d, 1));
    serviceList.push_back(make_pair(0x012e, 1));
    serviceList.push_back(make_pair(0x012f, 1));
    serviceList.push_back(make_pair(0x0130, 1));
    serviceList.push_back(make_pair(0x0131, 1));
    serviceListDes1.SetData(serviceList);
    bufferSize = serviceListDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    serviceListDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x41, buffer.get() + 2, bufferSize - 2); 

    cdsDes1.SetData(0x03220000, 0, 5, 0x68750, 0);
    bufferSize = cdsDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    cdsDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x44, buffer.get() + 2, bufferSize - 2); 

    uchar_t descriptor0833[] = 
    { 
        0x01, 0x2D, 0x00, 0x0F, 0x01, 0x2E, 0x00, 0x11, 0x01, 0x2F,
        0x00, 0x12, 0x01, 0x30, 0x00, 0x13, 0x01, 0x31, 0x00, 0x10
    };
    nit.AddTsDescriptor(tsId, onId, 0x83, descriptor0833, 20); 

    ///////////////////////////////////////
    tsId = 0x4;
    onId = 0x0;
    nit.AddTs(tsId, onId);

    serviceList.clear();
    serviceList.push_back(make_pair(0x0fa1, 1));
    serviceList.push_back(make_pair(0x0fa2, 1));
    serviceList.push_back(make_pair(0x0fa3, 1));
    serviceList.push_back(make_pair(0x0fa4, 1));
    serviceList.push_back(make_pair(0x0fa5, 1));
    serviceList.push_back(make_pair(0x0fa6, 1));
    serviceListDes1.SetData(serviceList);
    bufferSize = serviceListDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    serviceListDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x41, buffer.get() + 2, bufferSize - 2); 

    cdsDes1.SetData(0x03300000, 0, 5, 0x68750, 0);
    bufferSize = cdsDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    cdsDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x44, buffer.get() + 2, bufferSize - 2); 

    uchar_t descriptor0834[] = 
    { 
        0x0F, 0xA1, 0x00, 0x64, 0x0F, 0xA2, 0x00, 0x65, 0x0F, 0xA3, 0x00, 0x66, 0x0F, 0xA4, 0x00, 0x67,
        0x0F, 0xA5, 0x00, 0x68, 0x0F, 0xA6, 0x00, 0x69
    };
    nit.AddTsDescriptor(tsId, onId, 0x83, descriptor0834, 24); 

    ///////////////////////////////////////
    tsId = 0x5;
    onId = 0x0;
    nit.AddTs(tsId, onId);

    serviceList.clear();
    serviceList.push_back(make_pair(0x1389, 2));
    serviceList.push_back(make_pair(0x138a, 1));
    serviceList.push_back(make_pair(0x138b, 1));
    serviceList.push_back(make_pair(0x138c, 2));
    serviceList.push_back(make_pair(0x138d, 1));
    serviceList.push_back(make_pair(0x138e, 1));
    serviceList.push_back(make_pair(0x0005, 1));
    serviceList.push_back(make_pair(0x0008, 1));
    serviceList.push_back(make_pair(0x0009, 1));
    serviceList.push_back(make_pair(0x000a, 1));
    serviceListDes1.SetData(serviceList);
    bufferSize = serviceListDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    serviceListDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x41, buffer.get() + 2, bufferSize - 2); 

    cdsDes1.SetData(0x03380000, 0, 5, 0x68750, 0);
    bufferSize = cdsDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    cdsDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x44, buffer.get() + 2, bufferSize - 2); 

    uchar_t descriptor0835[] = 
    { 
        0x13, 0x89, 0x01, 0xF6, 0x13, 0x8A, 0x03, 0xED, 0x13, 0x8B, 0x03, 0xEA,
        0x13, 0x8C, 0x01, 0xF7, 0x13, 0x8D, 0x03, 0xEC, 0x13, 0x8E, 0x03, 0xEB, 0x00, 0x05, 0x03, 0xE9,
        0x00, 0x08, 0x03, 0xEE, 0x00, 0x09, 0x03, 0xEF, 0x00, 0x0A, 0x03, 0xF0
    };
    nit.AddTsDescriptor(tsId, onId, 0x83, descriptor0835, 40); 

    ///////////////////////////////////////
    tsId = 0x6;
    onId = 0x0;
    nit.AddTs(tsId, onId);

    serviceList.clear();
    serviceList.push_back(make_pair(0x0259, 2));
    serviceList.push_back(make_pair(0x025a, 1));
    serviceList.push_back(make_pair(0x025b, 2));
    serviceList.push_back(make_pair(0x025c, 2));
    serviceList.push_back(make_pair(0x025d, 2));
    serviceList.push_back(make_pair(0x025e, 2));
    serviceList.push_back(make_pair(0x025f, 2));
    serviceList.push_back(make_pair(0x0260, 2));
    serviceList.push_back(make_pair(0x0261, 1));
    serviceList.push_back(make_pair(0x0262, 2));
    serviceList.push_back(make_pair(0x0263, 2));
    serviceList.push_back(make_pair(0x0264, 2));
    serviceList.push_back(make_pair(0x0265, 1));
    serviceList.push_back(make_pair(0x0266, 1));
    serviceListDes1.SetData(serviceList);
    bufferSize = serviceListDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    serviceListDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x41, buffer.get() + 2, bufferSize - 2); 

    cdsDes1.SetData(0x03460000, 0, 5, 0x68750, 0);
    bufferSize = cdsDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    cdsDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x44, buffer.get() + 2, bufferSize - 2); 

    uchar_t descriptor0836[] = 
    { 
        0x02, 0x59, 0x01,
        0xF8, 0x02, 0x5A, 0x07, 0xD3, 0x02, 0x5B, 0x01, 0xFE, 0x02, 0x5C, 0x01, 0xFF, 0x02, 0x5D, 0x02,
        0x00, 0x02, 0x5E, 0x02, 0x01, 0x02, 0x5F, 0x01, 0xFA, 0x02, 0x60, 0x01, 0xF9, 0x02, 0x61, 0x07,
        0xD2, 0x02, 0x62, 0x01, 0xFB, 0x02, 0x63, 0x01, 0xFC, 0x02, 0x64, 0x01, 0xFD, 0x02, 0x65, 0x07,
        0xD1, 0x02, 0x66, 0x07, 0xD4
    };
    nit.AddTsDescriptor(tsId, onId, 0x83, descriptor0836, 56); 

    ///////////////////////////////////////
    tsId = 0x7;
    onId = 0x0;
    nit.AddTs(tsId, onId);

    serviceList.clear();
    serviceList.push_back(make_pair(0x1b59, 1));
    serviceList.push_back(make_pair(0x1b5a, 1));
    serviceList.push_back(make_pair(0x1b5b, 1));
    serviceList.push_back(make_pair(0x1b5c, 1));
    serviceList.push_back(make_pair(0x1b5d, 1));
    serviceListDes1.SetData(serviceList);
    bufferSize = serviceListDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    serviceListDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x41, buffer.get() + 2, bufferSize - 2); 

    cdsDes1.SetData(0x03540000, 0, 5, 0x68750, 0);
    bufferSize = cdsDes1.GetCodesSize();
    ptr = new uchar_t[bufferSize];
    buffer.reset(ptr, UcharDeleter());
    cdsDes1.MakeCodes(buffer.get(), bufferSize);
    nit.AddTsDescriptor(tsId, onId, 0x44, buffer.get() + 2, bufferSize - 2); 

    uchar_t descriptor0837[] = 
    { 
        0x1B,
        0x59, 0x07, 0xD5, 0x1B, 0x5A, 0x07, 0xD6, 0x1B, 0x5B, 0x07, 0xD7, 0x1B, 0x5C, 0x07, 0xD8, 0x1B,
        0x5D, 0x07, 0xD9
    };
    nit.AddTsDescriptor(tsId, onId, 0x83, descriptor0837, 20); 

    Ts ts;
    ts.SetTransportPriority(1);
    ts.SetContinuityCounter(5);

    size_t size = ts.GetCodesSize(nit);
    buffer.reset(new uchar_t[size], UcharDeleter());
    size_t ret = ts.MakeCodes(nit, buffer.get(), size);

    fstream file("D:/Temp/Temp.ts", ios_base::out  | ios::binary);
    file.write((char*)buffer.get(), size); 

    CPPUNIT_ASSERT(true);
} 

/**********************class NitCase**********************/
CPPUNIT_TEST_SUITE_REGISTRATION(NitCase);
void NitCase::setUp()
{
}

void NitCase::TestMakeCodes()
{
    Nit nit1, nit2;

    /* Test tableId, networkId, versionNumber */
    nit1.SetTableId(1);
    nit1.SetNetworkId(1);
    nit1.SetVersionNumber(1);

    nit2.SetTableId(1);
    nit2.SetNetworkId(1);
    nit2.SetVersionNumber(1);
    CPPUNIT_ASSERT(nit1 == nit2);

    nit2.SetTableId(2);
    CPPUNIT_ASSERT(nit1 != nit2);

    nit2.SetTableId(1);
    CPPUNIT_ASSERT(nit1 == nit2);

    nit2.SetNetworkId(2);
    CPPUNIT_ASSERT(nit1 != nit2);

    nit2.SetNetworkId(1);
    CPPUNIT_ASSERT(nit1 == nit2);

    nit2.SetVersionNumber(2);
    CPPUNIT_ASSERT(nit1 != nit2);

    nit2.SetVersionNumber(1);
    CPPUNIT_ASSERT(nit1 == nit2);

    /* Test descriptor */
    nit1.AddDescriptor(1, (uchar_t*)"12345", 5);
    CPPUNIT_ASSERT(nit1 != nit2);

    nit2.AddDescriptor(1, (uchar_t*)"12345", 6);
    CPPUNIT_ASSERT(nit1 != nit2);
} 

/**********************class ControllerCase**********************/
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

CPPUNIT_TEST_SUITE_REGISTRATION(TsCase);
void TsCase::setUp()
{
}

void TsCase::Trigger(const NitWrapper<Nit>& wrapper) const
{
    /* do nothing */
}

void TsCase::TestMakeCodes()
{

} 

CxxEndNameSpace
//EOF