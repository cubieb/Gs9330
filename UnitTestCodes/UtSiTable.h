#ifndef _UtSiTable_h_
#define _UtSiTable_h_

#include <cppunit/extensions/HelperMacros.h>

CxxBeginNameSpace(UnitTest)

#pragma pack(push, 1)
struct bouquet_association_section
{
    uchar_t  table_id:8;

    uint16_t section_syntax_indicator:1;
    uint16_t reserved_future_use1:1;
    uint16_t reserved1:2;
    uint16_t section_length:12;

    uint16_t bouquet_id:16;

    uchar_t reserved2:2;
    uchar_t version_number:5;
    uchar_t current_next_indicator:1;

    uchar_t section_number:8;
    uchar_t last_section_number:8;

    uint16_t reserved_future_use2:4;
    uint16_t bouquet_descriptors_length:12;
    //for(i=0;i<N;i++)
    //{
    //    descriptor()
    //}

    uint16_t reserved_future_use:4;
    uint16_t transport_stream_loop_length:12;
    //for(i=0;i<N;i++)
    //{
    //    uint16_ttransport_stream_id:16;
    //    uint16_toriginal_network_id:16;

    //    uint16_t reserved_future_use:4;
    //    uint16_t transport_descriptors_length:12;
    //    for(j=0;j<N;j++)
    //    {
    //        descriptor()
    //    }
    //}
    uint32_t CRC_32:32;
};

struct transport_stream
{
    //for(i=0;i<N;i++)
    //{
        uint16_t transport_stream_id:          16; // uimsbf
        uint16_t original_network_id:          16; //uimsbf
        uint16_t reserved_future_use:          4;  //bslbf
        uint16_t transport_descriptors_length: 12; //uimsbf
    //    for(j=0;j<N;j++)
    //    {
    //        //cable_delivery_system_descriptor() must be included.
    //        descriptor()  
    //    }
    //}
};

//sizeof(event_information_section) is 18
struct event_information_section
{
    uchar_t table_id: 8;

    uint16_t section_syntax_indicator: 1;
    uint16_t reserved_future_use: 1;
    uint16_t reserved1: 2;
    uint16_t section_length: 12;

    uint16_t service_id: 16;

    uchar_t reserved2: 2;
    uchar_t version_number: 5;
    uchar_t current_next_indicator: 1;

    uchar_t section_number: 8;
    uchar_t last_section_number: 8;
    uint16_t transport_stream_id: 16;
    uint16_t original_network_id: 16;
    uchar_t  segment_last_section_number: 8;
    uchar_t  last_table_id: 8;
    /*  ... ... */
    uint32_t CRC_32: 32; 
};

//sizeof(event_information_section_detail) is 12
struct event_information_section_detail
{
    //for (i=0;i<N;i++)
    //{
    uint16_t event_id:           16;       //uimsbf     --

    uint64_t start_time:         40;
    uint64_t duration:           24;

    uint16_t running_status:             3;  //uimsbf 
    uint16_t free_CA_mode:               1;  //bslbf
    uint16_t descriptors_loop_length:    12; //uimsbf        --
    //    for (j=0;j<N;j++)
    //    {
    //        descriptor()
    //    }
    //}
};

/*
    define struct service_description_section just for calculating fixed fields size.
*/
struct service_description_section
{
    uchar_t  table_id:                 8;       //uimsbf  -
    uint16_t section_syntax_indicator: 1;       //bslbf
    uint16_t reserved_future_use1:     1;       //bslbf
    uint16_t reserved1:                2;       //bslbf
    uint16_t section_length:          12;       //uimsbf  --
    uint16_t transport_stream_id:     16;       //uimsbf  --
    uchar_t  reserved2:                2;       //bslbf
    uchar_t  version_number:           5;       //uimsbf
    uchar_t  current_next_indicator:   1;       //bslbf   -
    uchar_t  section_number:           8;       //uimsbf  -
    uchar_t  last_section_number:      8;       //uimsbf  -
    uint16_t original_network_id:     16;       //uimsbf  --
    uchar_t  reserved_future_use2:     8;       //bslbf   -
    //for (i=0;i<N;i++)
    //{
    //    service_id 16 uimsbf                     --
    //    reserved_future_use3 6 bslbf
    //    EIT_schedule_flag 1 bslbf
    //    EIT_present_following_flag 1 bslbf       -
    //    running_status 3 uimsbf 
    //    free_CA_mode 1 bslbf
    //    descriptors_loop_length 12 uimsbf        --
    //    for (j=0;j<N;j++)
    //    {
    //        descriptor()
    //    }
    //}
    uint32_t CRC_32:                  32;       //rpchof ----
};

struct service_description_section_detail
{
    //for (i=0;i<N;i++)
    //{
    uint16_t service_id:           16;       //uimsbf     --

    uchar_t  reserved_future_use3: 6;        //bslbf
    uchar_t  EIT_schedule_flag:    1;        // bslbf
    uchar_t  EIT_present_following_flag: 1;  //bslbf       -

    uint16_t running_status:             3;  //uimsbf 
    uint16_t free_CA_mode:               1;  //bslbf
    uint16_t descriptors_loop_length:    12; //uimsbf        --
    //    for (j=0;j<N;j++)
    //    {
    //        descriptor()
    //    }
    //}
};

#pragma pack(pop)

#define MaxBatDesAndTsContentSize (MaxBatSectionLength - sizeof(bouquet_association_section))
//MaxEitEventContentSize = 4078
#define MaxEitEventContentSize (MaxEitSectionLength - sizeof(event_information_section))
#define MaxEitEventDescriptorSize (MaxEitEventContentSize - sizeof(event_information_section_detail))
#define MaxSdtServiceContentSize (MaxSdtSectionLength - sizeof(service_description_section))

#define InvalidSiTableTableId 0

/**********************class SiTable**********************/
class SiTable : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(SiTable);
    /* Bat */
    CPPUNIT_TEST(TestBatConstruct);
    CPPUNIT_TEST(TestBatGetCodesSize);
    CPPUNIT_TEST(TestBatGetKey);
    CPPUNIT_TEST(TestBatGetSecNumber);
    CPPUNIT_TEST(TestBatGetTableId);
    CPPUNIT_TEST(TestBatMakeCodes);
    /* Eit */
    CPPUNIT_TEST(TestEitGetCodesSize);
    CPPUNIT_TEST(TestEitMakeCodes1);
    CPPUNIT_TEST(TestEitMakeCodes2);    
    /* Nit */
    CPPUNIT_TEST(TestNitMakeCodes);
    /* Sdt */
    CPPUNIT_TEST(TestSdtConstruct);
    CPPUNIT_TEST(TestSdtGetCodesSize);
    CPPUNIT_TEST(TestSdtGetKey);
    CPPUNIT_TEST(TestSdtGetSecNumber);
    CPPUNIT_TEST(TestSdtGetTableId);
    CPPUNIT_TEST(TestSdtMakeCodes);
    CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{}

protected:
    /* Bat */
    void TestBatConstruct();
    void TestBatGetCodesSize();
    void TestBatGetKey();
    void TestBatGetSecNumber();
    void TestBatGetTableId();
    void TestBatMakeCodes();
    /* Eit */
    void TestEitGetCodesSize();
    void TestEitMakeCodes1();
    void TestEitMakeCodes2();
    /* Nit */
    void TestNitMakeCodes();
    /* Sdt */
    void TestSdtConstruct();
    void TestSdtGetCodesSize();
    void TestSdtGetKey();
    void TestSdtGetSecNumber();
    void TestSdtGetTableId();
    void TestSdtMakeCodes();
};

CxxEndNameSpace
#endif