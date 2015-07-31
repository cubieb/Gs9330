#ifndef _UnitTest_h_
#define _UnitTest_h_

#include <cppunit/extensions/HelperMacros.h>

CxxBeginNameSpace(UnitTest)
/* 
 * A test case that is designed to produce
 * example errors and failures
 *
 */

class XmlDataWrapperTestCase : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( XmlDataWrapperTestCase );
    CPPUNIT_TEST( TestReadXmlTalbe );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();

protected:
    void TestReadXmlTalbe();
};

CxxEndNameSpace
#endif