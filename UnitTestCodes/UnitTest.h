#ifndef _UnitTest_h_
#define _UnitTest_h_

#include <cppunit/extensions/HelperMacros.h>

CxxBeginNameSpace(UnitTest)

/**********************class XmlDataWrapperTestCase**********************/
class XmlDataWrapperTestCase : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(XmlDataWrapperTestCase);
    CPPUNIT_TEST(TestTrigger);
    CPPUNIT_TEST(TestFillNit);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();

protected:    
    void TestTrigger();
    void TestFillNit();

private:
    void Trigger(const NitWrapper<Nit>& wrapper) const;
};

/**********************class CrcTestCase**********************/
class CrcTestCase : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(CrcTestCase);
    CPPUNIT_TEST(TestCrc32);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();

protected:
    void TestCrc32();
};

/**********************class NitCase**********************/
class NitCase : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(NitCase);
    CPPUNIT_TEST(TestMakeCodes);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();

protected:
    void TestMakeCodes();
};

/**********************class TsCase**********************/
class TsCase : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TsCase);
    CPPUNIT_TEST(TestMakeCodes);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();

protected:
    void TestMakeCodes();

private:
    void Trigger(const NitWrapper<Nit>& wrapper) const;
};


CxxEndNameSpace
#endif