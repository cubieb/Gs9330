#ifndef _Example_h_
#define _Example_h_

#include <cppunit/extensions/HelperMacros.h>

CxxBeginNameSpace(UnitTest)

/**********************Example**********************/
class Example : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Example);
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