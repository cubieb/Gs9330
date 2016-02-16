#ifndef _UtMpegTransportStream_h_
#define _UtMpegTransportStream_h_

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

/**********************class TransportPacket**********************/
class TransportPacket : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TransportPacket);
    CPPUNIT_TEST(TestTransportPacketConstruct);
    CPPUNIT_TEST(TestTransportPacketAddSiTable);
    CPPUNIT_TEST(TestTransportPacketDelSiTable);
    CPPUNIT_TEST(TestTransportPacketFindSiTable);
    CPPUNIT_TEST(TestTransportPacketGetCodesSize);
    CPPUNIT_TEST(TestTransportPacketGetNetId);
    CPPUNIT_TEST(TestTransportPacketGetPid);
    CPPUNIT_TEST(TestTransportPacketMakeCodes1);
    CPPUNIT_TEST(TestTransportPacketMakeCodes2);
    CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{}

protected:
    void TestTransportPacketConstruct();
    void TestTransportPacketAddSiTable();
    void TestTransportPacketDelSiTable();
    void TestTransportPacketFindSiTable();
    void TestTransportPacketGetCodesSize();
    void TestTransportPacketGetNetId();
    void TestTransportPacketGetPid();
    void TestTransportPacketMakeCodes1();
    void TestTransportPacketMakeCodes2();
};

/**********************class TransportPackets**********************/
class TransportPackets : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TransportPackets);
    CPPUNIT_TEST(TestTransportPacketsBegin);
    CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{}

protected:
    void TestTransportPacketsBegin();
};

CxxEndNameSpace
#endif