#include "Include/Foundation/SystemInclude.h"
#include <cppunit/extensions/HelperMacros.h>

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Converter.h"

#include "UtConverter.h"
using namespace std;

CxxBeginNameSpace(UnitTest)
    
/**********************Converter**********************/
CPPUNIT_TEST_SUITE_REGISTRATION(Converter);

void Converter::TestConvertStrToIntStr()
{
    char *src = "1112";
    uchar_t dst[32];
    size_t size = ConvertStrToIntStr((uchar_t*)src, 4, dst);

	CPPUNIT_ASSERT(size == 2 && dst[0] == 0x11 && dst[1] == 0x12);
}

void Converter::TestConvertUtf8ToString()
{
    char *src = "\xe4\xb8\x80\xe4\xb8\x81\xe4\xb8\x83\xe4\xb8\x87";  //一丁七万
    string dst;
    char *gb2312 = "\xD2\xBB\xB6\xA1\xC6\xDF\xCD\xF2";

    ConvertUtf8ToString((uchar_t*)src, dst, false);
    CPPUNIT_ASSERT(memcmp(dst.c_str(), gb2312, 8) == 0);
    CPPUNIT_ASSERT(dst == string("一丁七万"));

    ConvertUtf8ToString((uchar_t*)src, dst, true);    
	CPPUNIT_ASSERT(dst == string("\x13一丁七万"));
}

CxxEndNameSpace
