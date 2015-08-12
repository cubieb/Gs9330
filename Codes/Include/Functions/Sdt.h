#ifndef _Sdt_h_
#define _Sdt_h_

#include "SectionBase.h"

/*
uimsbf:  unsigned integer most significant bit first
bslbf :  bit string, left bit first
*/
#pragma pack(push, 1)

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
    uchar_t  current_next_indicator:   1;       //bslbf
    uchar_t  section_number:           8;       //uimsbf  -
    uchar_t  last_section_number:      8;       //uimsbf  -
    uint16_t original_network_id:     16;       //uimsbf  --
    uchar_t  reserved_future_use2:     8;       //bslbf   -
    //for (i=0;i<N;i++)
    //{
    //    service_id 16 uimsbf
    //    reserved_future_use3 6 bslbf
    //    EIT_schedule_flag 1 bslbf
    //    EIT_present_following_flag 1 bslbf
    //    running_status 3 uimsbf
    //    free_CA_mode 1 bslbf
    //    descriptors_loop_length 12 uimsbf
    //    for (j=0;j<N;j++)
    //    {
    //        descriptor()
    //    }
    //}
    uint32_t CRC_32:                  32;       //rpchof
};
#pragma pack(pop)

/**********************class Sdt**********************/
/* 5.2.3 Service Description Table */
class Sdt: public SectionBase
{
public:
    enum: uint16_t {Pid = 0x0011};
    Sdt();
    ~Sdt() {}
    
    uint16_t GetPid()  const; 

    void SetTableId(uchar_t data);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uchar_t  tableId;
    uint16_t transportStreamId;
    uint16_t originalNetworkId;
};

#endif