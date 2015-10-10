#ifndef _Nit_h_
#define _Nit_h_

#include "Component.h"

/*
    uimsbf:  unsigned integer most significant bit first
    bslbf :  bit string, left bit first
*/
#pragma pack(push, 1)

/*
define struct network_information_section just for calculating fixed fields size.
sub_table: collection of sections with the same value of table_id and:
    for a NIT: the same table_id_extension (network_id) and version_number;
    for a BAT: the same table_id_extension (bouquet_id) and version_number;
    for a SDT: the same table_id_extension (transport_stream_id), the same original_network_id and version_number;
    for a EIT: the same table_id_extension (service_id), the same transport_stream_id, 
               the same original_network_id and version_number.
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
    //uint16_t reserved_future_use3:4;              // 4 bslbf   
    //uint16_t transport_stream_loop_length:12;     // 12 uimsbf --
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

class Descriptors;
class TransportStreams;

/**********************class Nit**********************/
/* 5.2.1 Network Information Table */
class Nit: public Section
{
public:
    /* SI PID definition: <iso13818-1.pdf>, 5.1.3 Coding of PID and table_id fields */
    enum: uint16_t {Pid = 0x0010};
    Nit(const char *key);
    Nit(const char *key, uchar_t *buffer);
    ~Nit() {}
    
    uint16_t GetSectionId() const;
    uint16_t GetPid() const; 

    void SetTableId(uchar_t data);
    uchar_t GetTableId() const;

    void SetNetworkId(uint16_t data);
    uint16_t GetNetworkId() const;

    void SetVersionNumber(uchar_t data);
    void SetSectionNumber(uchar_t data);
    void SetLastSectionNumber(uchar_t data);

    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    void AddDescriptor0x41(const std::list<std::pair<uint16_t, uchar_t>>& serviceList);
    void AddDescriptor0x44(uint32_t frequency, uint16_t fecOuter, uchar_t modulation,
                           uint32_t symbolRate, uint32_t fecInner);

    void AddTs(uint16_t tsId, uint16_t onId);
    void AddTsDescriptor(uint16_t tsId, uchar_t tag, uchar_t* data, size_t dataSize);    
    void AddTsDescriptor0x41(uint16_t tsId,
                             const std::list<std::pair<uint16_t, uchar_t>>& serviceList);
    void AddTsDescriptor0x44(uint16_t tsId,
                             uint32_t frequency, uint16_t fecOuter, uchar_t modulation,
                             uint32_t symbolRate, uint32_t fecInner);
    
    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uchar_t  tableId;
    uint16_t networkId;
    uchar_t  versionNumber;
    uchar_t  sectionNumber;
    uchar_t  lastSectionNumber;
    std::shared_ptr<Descriptors> descriptors;
    std::shared_ptr<TransportStreams> transportStreams;
};

#endif