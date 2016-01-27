#ifndef _ExitProcessTest_h_
#define _ExitProcessTest_h_

#include <cppunit/extensions/HelperMacros.h>

CxxBeginNameSpace(UnitTest)

/**********************Xxx**********************/
class Xxx : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Xxx);
    CPPUNIT_TEST(TestXxx);
    CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{}

protected:
    void TestXxx();
};

CxxEndNameSpace
#endif