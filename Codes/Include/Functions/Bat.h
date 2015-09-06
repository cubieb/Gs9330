#ifndef _Bat_h_
#define _Bat_h_

#include "Component.h"

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

class Bat: public Section
{
public:
    /* SI PID definition: <iso13818-1.pdf>, 5.1.3 Coding of PID and table_id fields */
    enum: uint16_t {Pid = 0x0011};
    Bat();
    ~Bat() {}
    
    uint16_t GetPid()  const; 

    void SetTableId(uchar_t data);
    uchar_t GetTableId() const;

    void SetNetworkId(uint16_t data);
    uint16_t GetNetworkId() const;

    void SetBouquetId(uint16_t data);
    void SetVersionNumber(uchar_t data);

    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    void AddTs(uint16_t tsId, uint16_t onId);
    void AddTsDescriptor(uint16_t tsId, uchar_t tag, uchar_t* data, size_t dataSize);   
    void AddTsDescriptor0x41(uint16_t tsId,
                             const std::list<std::pair<uint16_t, uchar_t>>& serviceList);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uchar_t  tableId;
    //protocol do not define network Id for this section, we use this networkId to
    //determine whether the current section should be send to a sub-network
    uint16_t networkId;   
    uint16_t bouquetId;
    uchar_t  versionNumber;
    uchar_t  sectionNumber;
    uchar_t  lastSectionNumber;
    std::shared_ptr<Descriptors> descriptors;
    std::shared_ptr<TransportStreams> transportStreams;
};

#endif