#ifndef _UtNetworksCfg_h_
#define _UtNetworksCfg_h_

#include <cppunit/extensions/HelperMacros.h>

CxxBeginNameSpace(UnitTest)

/**********************NetworksCfg**********************/
class NetworksCfg : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(NetworksCfg);
    CPPUNIT_TEST(TestReceiver); 
    CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{}

protected:
    void TestReceiver();
};

CxxEndNameSpace
#endif