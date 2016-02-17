#ifndef _UtConverter_h_
#define _UtConverter_h_

#include <cppunit/extensions/HelperMacros.h>

CxxBeginNameSpace(UnitTest)

/**********************Converter**********************/
class Converter : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Converter);
    CPPUNIT_TEST(TestConvertStrToIntStr);    
    CPPUNIT_TEST(TestConvertUtf8ToString);
    CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{}

protected:
    void TestConvertStrToIntStr();
    void TestConvertUtf8ToString();
};

CxxEndNameSpace
#endif