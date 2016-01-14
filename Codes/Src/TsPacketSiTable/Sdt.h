#ifndef _Sdt_h_
#define _Sdt_h_

#include "Descriptor.h"       //Descriptor 
#include "TransportStream.h"  //TransportStream

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

/**********************class SdtService**********************/
class SdtService
{
public:
    SdtService(ServiceId serviceId, uchar_t eitScheduleFlag, uchar_t eitPresentFollowingFlag,
               uint16_t runningStatus, uint16_t freeCaMode);
    ~SdtService();

    void AddDescriptor(Descriptor *descriptor);

    size_t GetCodesSize() const;
    ServiceId GetServiceId() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

private:    
    ServiceId serviceId; 
    uchar_t  eitScheduleFlag;
    uchar_t  eitPresentFollowingFlag; 
    uint16_t runningStatus; 
    uint16_t freeCaMode;
    Descriptors descriptors;
};

class CompareSdtServiceId: std::unary_function<SdtService, bool>
{
public:
    CompareSdtServiceId(ServiceId serviceId)
        : serviceId(serviceId)
    {}

    result_type operator()(const argument_type &sdtService)
    {
        return (result_type)(sdtService.GetServiceId() == serviceId);
    }

    result_type operator()(const argument_type *sdtService)
    {
        return this->operator()(*sdtService);
    }

private:
    uint16_t serviceId;
};

/**********************class SdtServices**********************/
class SdtServices
{
public:
    SdtServices();
    ~SdtServices();

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    void AddSdtService(SdtService* service);
    void AddServiceDescriptor(ServiceId serviceId, Descriptor *descriptor);

private:
    std::list<SdtService*> sdtServices;
};

/**********************class SdtTable**********************/
class SdtTable: public SdtTableInterface
{
public:
    SdtTable(TableId tableId, TsId transportStreamId, Version versionNumber, NetId originalNetworkId);
    ~SdtTable();    

    void AddService(ServiceId serviceId, uchar_t eitScheduleFlag, 
                    uchar_t eitPresentFollowingFlag, uint16_t runningStatus, uint16_t freeCaMode);
    void AddServiceDescriptor(ServiceId serviceId, std::string &data);

    size_t GetCodesSize(TableId tableId, const std::list<TsId>& tsIds) const;
    uint16_t GetKey() const;
    TableId GetTableId() const;
    size_t MakeCodes(TableId tableId, std::list<TsId>& tsIds, 
                     uchar_t *buffer, size_t bufferSize) const;

private:
    TableId  tableId;    
    TsId transportStreamId;
    Version  versionNumber;
    uchar_t  sectionNumber;
    uchar_t  lastSectionNumber;
    NetId originalNetworkId;

    SdtServices sdtServices;
};

#pragma pack(pop)
#endif