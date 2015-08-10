#ifndef _DataWrapper_h_
#define _DataWrapper_h_

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
    uint32_t table_id:8;                        // 8 uimsbf  -
    uint32_t section_syntax_indicator:1;        // 1 bslbf    
    uint32_t reserved_future_use1:1;            // 1 bslbf    
    uint32_t reserved1:2;                       // 2 bslbf    
    uint32_t section_length:12;                 // 12 uimsbf -- 
    uint32_t network_id:16;                     // 16 uimsbf -- 
    uint32_t reserved2:2;                       // 2 bslbf     
    uint32_t version_number:5;                  // 5 uimsbf    
    uint32_t current_next_indicator:1;          // 1 bslbf   -  
    uint32_t section_number:8;                  // 8 uimsbf  - 
    uint32_t last_section_number:8;             // 8 uimsbf  -
    //uint32_t reserved_future_use2:4;            // 4 bslbf   
    //uint32_t network_descriptors_length:12;     // 12 uimsbf  --
    //for(i=0;i<N;i++)
    //{
    //    descriptor()
    //}
    uint32_t reserved_future_use3:4;              // 4 bslbf   
    uint32_t transport_stream_loop_length:12;     // 12 uimsbf --
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
    uint32_t CRC_32;                           //32 rpchof
};
#pragma pack(pop)

#define MaxNitSectionLength     1024

#define SectionSyntaxIndicator  0x1
#define Reserved1Bit            0x1
#define Reserved2Bit            0x3
#define Reserved4Bit            0xF

template<typename Nit>
class NitWrapper
{
public:
    typedef NitWrapper<Nit> MyType;
    typedef std::function<void(const MyType&)> Trigger;

    NitWrapper(Trigger& theTrigger)
        : trigger(theTrigger)
    {}
    
    virtual ~NitWrapper() 
    {}
    virtual std::error_code FillNit(Nit& nit) const = 0;

private:
    Trigger trigger;
};

#endif