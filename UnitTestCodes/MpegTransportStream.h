#ifndef _MpegTransportStream_h_
#define _MpegTransportStream_h_

#include <cppunit/extensions/HelperMacros.h>

CxxBeginNameSpace(UnitTest)
#pragma pack(push, 1)
/*
    define struct transport_packet just for calculating fixed fields size.
*/
struct transport_packet
{
    uchar_t  sync_byte:8;                       // 8 uimsbf  -
    uint16_t transport_error_indicator:1;       // 1 bslbf    
    uint16_t payload_unit_start_indicator:1;    // 1 bslbf    
    uint16_t transport_priority:1;              // 1 bslbf
    uint16_t PID:13;                            // 13 bslbf  --
    uchar_t  transport_scrambling_control:2;    // 2 bslbf
    uchar_t  adaptation_field_control:2;        // 2 bslbf
    uchar_t  continuity_counter:4;              // 4 bslbf   -
};
#pragma pack(pop)
#define MaxTsPacketPayloadSize (TsPacketSize - sizeof(transport_packet))

#define InvalidPid 0

/**********************class MpegTransportStream**********************/
class MpegTransportStream : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(MpegTransportStream);
    CPPUNIT_TEST(TestTsPacketConstruct);
    CPPUNIT_TEST(TestTsPacketAddSiTable);
    CPPUNIT_TEST(TestTsPacketDelSiTable);
    CPPUNIT_TEST(TestTsPacketFindSiTable);
    CPPUNIT_TEST(TestTsPacketGetCodesSize);
    CPPUNIT_TEST(TestTsPacketGetNetId);
    CPPUNIT_TEST(TestTsPacketGetPid);
    CPPUNIT_TEST(TestTsPacketMakeCodes1);    
    CPPUNIT_TEST(TestTsPacketMakeCodes2);
    CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{}

protected:
    void TestTsPacketConstruct();
    void TestTsPacketAddSiTable();
    void TestTsPacketDelSiTable();
    void TestTsPacketFindSiTable();
    void TestTsPacketGetCodesSize();
    void TestTsPacketGetNetId();
    void TestTsPacketGetPid();
    void TestTsPacketMakeCodes1();
    void TestTsPacketMakeCodes2();
};

CxxEndNameSpace
#endif