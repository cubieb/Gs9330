#ifndef _UtSiTableXmlWrapperInterface_h_
#define _UtSiTableXmlWrapperInterface_h_

#include <cppunit/extensions/HelperMacros.h>

CxxBeginNameSpace(UnitTest)

/**********************class SiTableXmlWrapper**********************/
class SiTableXmlWrapper : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(SiTableXmlWrapper);
    CPPUNIT_TEST(TestBatXmlWrapperSelect);
    CPPUNIT_TEST(TestEitXmlWrapperSelect);
    CPPUNIT_TEST(TestNitXmlWrapperSelect);
    CPPUNIT_TEST(TestSdtXmlWrapperSelect);
    CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{}

protected:
    void TestBatXmlWrapperSelect();
    void TestEitXmlWrapperSelect();
    void TestNitXmlWrapperSelect();
    void TestSdtXmlWrapperSelect();

private:
    void TestBatXmlWrapperSelect01();
    void TestBatXmlWrapperSelect02();
};

CxxEndNameSpace
#endif