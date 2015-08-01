#include "SystemInclude.h"
#include "Common.h"
#include "Crc32.h"

#include "UnitTest.h"

using namespace std;

CxxBeginNameSpace(UnitTest)

CPPUNIT_TEST_SUITE_REGISTRATION( XmlDataWrapperTestCase );
void XmlDataWrapperTestCase::setUp()
{
} 

void XmlDataWrapperTestCase::TestReadXmlTalbe()
{
    CPPUNIT_ASSERT(1 == 1);
} 

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
    CPPUNIT_ASSERT(crc == 0x4c2750bd);

    crc = crc32.FileCrc("D:/Temp/tmp.txt");
    CPPUNIT_ASSERT(crc == 0x4c2750bd);
} 

CxxEndNameSpace
//EOF