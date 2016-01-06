#ifndef _Eit_h_
#define _Eit_h_

#pragma pack(push, 1)
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

#pragma pack(pop)
#endif