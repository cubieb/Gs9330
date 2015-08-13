#ifndef _Nit_h_
#define _Nit_h_

#include "Types.h"
#include "Descriptor.h"
#include "SectionBase.h"

/*
uimsbf:  unsigned integer most significant bit first
bslbf :  bit string, left bit first
*/
#pragma pack(push, 1)

/*
    define struct network_information_section just for calculating fixed fields size.
*/
struct network_information_section
{
    uchar_t  table_id:8;                        // 8 uimsbf  -
    uint16_t section_syntax_indicator:1;        // 1 bslbf    
    uint16_t reserved_future_use1:1;            // 1 bslbf    
    uint16_t reserved1:2;                       // 2 bslbf    
    uint16_t section_length:12;                 // 12 uimsbf -- 
    uint16_t network_id:16;                     // 16 uimsbf -- 
    uchar_t  reserved2:2;                       // 2 bslbf     
    uchar_t  version_number:5;                  // 5 uimsbf    
    uchar_t  current_next_indicator:1;          // 1 bslbf   -  
    uchar_t  section_number:8;                  // 8 uimsbf  - 
    uchar_t  last_section_number:8;             // 8 uimsbf  -
    //uint32_t reserved_future_use2:4;            // 4 bslbf   
    //uint32_t network_descriptors_length:12;     // 12 uimsbf --
    //for(i=0;i<N;i++)
    //{
    //    descriptor()
    //}
    uint16_t reserved_future_use3:4;              // 4 bslbf   
    uint16_t transport_stream_loop_length:12;     // 12 uimsbf --
    //for(i=0;i<N;i++)
    //{
    //    transport_stream_id          16 uimsbf
    //    original_network_id          16 uimsbf
    //    reserved_future_use          4 bslbf
    //    transport_descriptors_length 12 uimsbf
    //    for(j=0;j<N;j++)
    //    {
    //        //cable_delivery_system_descriptor() must be included.
    //        descriptor()  
    //    }
    //}
    uint32_t CRC_32;                           //32 rpchof  ----
};
#pragma pack(pop)

class Descriptor;
class Descriptors;
/**********************class Nit**********************/
/* 5.2.1 Network Information Table */
class Nit: public SectionBase
{
public:
    enum: uint16_t {Pid = 0x0010};
    typedef TransportStream TransportStream;
    Nit();
    ~Nit() {}
    
    uint16_t GetPid()  const; 

    void SetTableId(uchar_t data);
    void SetNetworkId(uint16_t data);
    void SetVersionNumber(uchar_t data);

    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    TransportStream& AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uchar_t  tableId;
    uint16_t networkId;
    uchar_t  versionNumber;
    Descriptors descriptors;
    std::list<std::shared_ptr<TransportStream>> transportStreams;
};

#endif