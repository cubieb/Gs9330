#ifndef _Nit_h_
#define _Nit_h_

#include "Descriptor.h"       //Descriptor 
#include "TransportStream.h"  //TransportStream

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

/**********************class NitTable**********************/
class NitTable: public NitTableInterface
{
public:
    NitTable(TableId tableId, NetId networkId, Version versionNumber);
    ~NitTable();    

    void AddDescriptor(std::string &data);
    void AddTs(TsId tsId, OnId onId);
    void AddTsDescriptor(TsId tsId, std::string &data);

    size_t GetCodesSize(TableId tableId, const std::list<TsId>& tsIds) const;
    uint16_t GetKey() const;
    TableId GetTableId() const;
    size_t MakeCodes(TableId tableId, std::list<TsId>& tsIds, 
                     uchar_t *buffer, size_t bufferSize) const;

private:
    TableId tableId;
    NetId networkId;
    Version versionNumber;
    SectionNumber sectionNumber;
    SectionNumber lastSectionNumber;
    TransportStreams transportStreams;
    Descriptors descriptors;
};

#endif