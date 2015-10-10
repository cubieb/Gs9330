#ifndef _Sdt_h_
#define _Sdt_h_

#include "Component.h"

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
#pragma pack(pop)

/**********************class SdtService**********************/
class SdtService: public Component
{
public:
    SdtService(uint16_t serviceId, uchar_t eitScheduleFlag, 
        uchar_t eitPresentFollowingFlag, uint16_t runningStatus, 
        uint16_t freeCaMode);
    
    uint16_t GetServiceId();

    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    void AddServiceDescriptor0x48(uchar_t serviceType, uchar_t *providerName, uchar_t *serviceName);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;   

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uint16_t serviceId; 
    uchar_t  eitScheduleFlag;
    uchar_t  eitPresentFollowingFlag; 
    uint16_t runningStatus; 
    uint16_t freeCaMode;
    std::shared_ptr<Descriptors> descriptors;
};

class EqualSdtService: std::unary_function<const std::shared_ptr<Component>&, bool>
{
public:
    EqualSdtService(uint16_t serviceId)
        : serviceId(serviceId)
    {}

    result_type operator()(argument_type component)
    {
        SdtService& service = dynamic_cast<SdtService&>(*component);
        return (result_type)(service.GetServiceId() == serviceId);
    }

private:
    uint16_t serviceId;
};

/**********************class SdtServices**********************/
class SdtServices: public Components
{    
public:
    typedef Components MyBase;

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    void AddSdtService(uint16_t serviceId, uchar_t eitScheduleFlag, 
        uchar_t eitPresentFollowingFlag, uint16_t runningStatus, 
        uint16_t freeCaMode);
    void AddServiceDescriptor(uint16_t serviceId, uchar_t tag, uchar_t* data, size_t dataSize);
    void AddServiceDescriptor0x48(uint16_t serviceId, uchar_t serviceType, 
                                  uchar_t *providerName, uchar_t *serviceName);

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;
};

/**********************class Sdt**********************/
/* 5.2.3 Service Description Table */
class Sdt: public Section
{
public:
    /* SI PID definition: <iso13818-1.pdf>, 5.1.3 Coding of PID and table_id fields */
    enum: uint16_t {Pid = 0x0011};
    Sdt(const char *key);
    Sdt(const char *key, uchar_t *buffer);
    ~Sdt() {}
    
    uint16_t GetSectionId() const;
    uint16_t GetPid()  const; 

    void SetTableId(uchar_t data);
    uchar_t GetTableId() const;
    
    void SetNetworkId(uint16_t data);
    uint16_t GetNetworkId() const;

    void SetTsId(uint16_t data);
    void SetVersionNumber(uchar_t data);
    void SetSectionNumber(uchar_t data);
    void SetLastSectionNumber(uchar_t data);
    void SetOnId(uint16_t data);

    void AddService(uint16_t serviceId, uchar_t eitScheduleFlag, 
        uchar_t eitPresentFollowingFlag, uint16_t runningStatus, uint16_t freeCaMode);
    void AddServiceDescriptor(uint16_t serviceId, uchar_t tag, uchar_t* data, size_t dataSize);
    void AddServiceDescriptor0x48(uint16_t serviceId, uchar_t serviceType,
                                  uchar_t *providerName, uchar_t *serviceName);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uchar_t  tableId;    
    //protocol do not define network Id for this section, we use this networkId to
    //determine whether the current section should be send to a sub-network
    uint16_t networkId; 
    uint16_t transportStreamId;
    uchar_t  versionNumber;
    uchar_t  sectionNumber;
    uchar_t  lastSectionNumber;
    uint16_t originalNetworkId;

    std::shared_ptr<SdtServices> services;
};

#endif