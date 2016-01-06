#ifndef _Bat_h_
#define _Bat_h_

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

    //uint16_t reserved_future_use2:4;
    //uint16_t bouquet_descriptors_length:12;
    //for(i=0;i<N;i++)
    //{
    //    descriptor()
    //}

    //uint16_t reserved_future_use:4;
    //uint16_t transport_stream_loop_length:12;
    //
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

#pragma pack(pop)
#endif