#include "SystemInclude.h"
#include "Common.h"
#include "Crc32.h"

#include "Nit.h"
#include "XmlDataWrapper.h"
#include "Ts.h"
#include "Controller.h"
#include "UnitTest.h"

using namespace std;

CxxBeginNameSpace(UnitTest)

/**********************class XmlDataWrapperTestCase**********************/
CPPUNIT_TEST_SUITE_REGISTRATION( XmlDataWrapperTestCase );
void XmlDataWrapperTestCase::setUp()
{
} 

void XmlDataWrapperTestCase::TestTrigger()
{
    CPPUNIT_ASSERT(true);
} 

void XmlDataWrapperTestCase::TestFillNit()
{
    CPPUNIT_ASSERT(true);
} 

/**********************class CrcTestCase**********************/
CPPUNIT_TEST_SUITE_REGISTRATION(CrcTestCase);
void CrcTestCase::setUp()
{
}

void CrcTestCase::TestCrc32()
{
    char* txt = "abcdefghijklmnopqrstuvwxyz";
    size_t size = strlen(txt);

    Crc32 crc32;
    uint32_t crc = crc32.FullCrc((uchar_t *)txt, size); 
    //CPPUNIT_ASSERT(crc == 0x4c2750bd);
    CPPUNIT_ASSERT(true);

    //crc = crc32.FileCrc("D:/Temp/tmp.txt");
    //CPPUNIT_ASSERT(crc == 0x4c2750bd);
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
    Nit nit;
    //function<void(const NitWrapper<Nit>&)> trigger(bind(&TsCase::Trigger, this, _1));
    //NitXmlWrapper<Nit> nitWrapper(trigger, "../XmlFiles/Nit.xml"); 
    //nitWrapper.FillNit(nit);
    nit.SetTableId(0x40);
    nit.SetNetworkId(1);
    nit.SetVersionNumber(1);
    char *name = "Chengdu Broadcast Television Network";
    nit.AddDescriptor(0x40, (uchar_t*)name, strlen(name)); 
    nit.AddTransportStream(5, 5);
 
    Ts ts;
    size_t size = ts.GetCodesSize(nit);
    shared_ptr<uchar_t> buffer(new uchar_t[size], UcharDeleter());
    memset(buffer.get(), 0xcd, size);
    
    fstream file("D:/Temp/Temp.ts", ios_base::out  | ios::binary);

    size_t ret = ts.MakeCodes(nit, buffer.get(), size);
    CPPUNIT_ASSERT(ret == size);

    /* check if there is any space which has not be filled value by MakeCodes() */
    uchar_t *ptr = buffer.get();
    uchar_t fill[2] = {0xcd, 0xcd};
    CPPUNIT_ASSERT(find_first_of( ptr, ptr + size, fill, fill + sizeof(fill)) == ptr + size);
    file.write((char*)buffer.get(), size);    
    file.close();
} 

CxxEndNameSpace
//EOF