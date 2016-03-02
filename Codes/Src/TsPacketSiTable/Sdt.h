#ifndef _Sdt_h_
#define _Sdt_h_

#include "Descriptor.h"       //Descriptor 
#include "TransportStream.h"  //TransportStream
#include "SiTableTemplate.h"

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

struct service_description_section_detail
{
    //for (i=0;i<N;i++)
    //{
    uint16_t service_id:           16;       //uimsbf     --

    uchar_t  reserved_future_use3: 6;        //bslbf
    uchar_t  EIT_schedule_flag:    1;        // bslbf
    uchar_t  EIT_present_following_flag: 1;  //bslbf       -

    uint16_t running_status:             3;  //uimsbf 
    uint16_t free_CA_mode:               1;  //bslbf
    uint16_t descriptors_loop_length:    12; //uimsbf        --
    //    for (j=0;j<N;j++)
    //    {
    //        descriptor()
    //    }
    //}
};
#pragma pack(pop)
#define SdtFixedFieldSize sizeof(service_description_section)
#define MaxSdtServiceContentSize (MaxSdtSectionLength - SdtFixedFieldSize)

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
class SdtServices: public ContainerBase
{
public:
    SdtServices();
    ~SdtServices();
    
    void AddSdtService(SdtService* service);
    void AddServiceDescriptor(ServiceId serviceId, Descriptor *descriptor);
    
    // ContainerBase function. construct proxy from _Alnod
    void AllocProxy()
    {
        myProxy = new ContainerProxy;
        myProxy->myContainer = this;
    }
    // ContainerBase function, destroy proxy.
    void FreeProxy()
    {
        OrphanAll();
        delete myProxy;
        myProxy = nullptr;
    }

    size_t GetCodesSize(size_t maxSize, size_t &offset) const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize, size_t offset) const;

private:
    std::list<SdtService*> sdtServices;
};

template<typename SdtServices>
class SdtServicesBinder: public IteratorBase 
{
public:
    SdtServicesBinder(const SdtServices &sdtServices)
        : sdtServices(sdtServices)
    {
        const ContainerBase *container = &sdtServices;
        this->Adopt(container);
    }

    size_t GetCodesSize(size_t maxSize, size_t offset) const
    {
#ifdef _DEBUG
        assert(this->GetContainer() != nullptr);
#endif
        return sdtServices.GetCodesSize(maxSize, offset);
    }

    size_t MakeCodes(uchar_t *buffer, size_t bufferSize, size_t offset) const
    {
#ifdef _DEBUG
        assert(this->GetContainer() != nullptr);
#endif
        return sdtServices.MakeCodes(buffer, bufferSize, offset);
    }

private:
    const SdtServices &sdtServices;
};

/**********************class SdtTable**********************/
class SdtTable: public SiTableTemplate<VarHelper, SdtServicesBinder<SdtServices>>
{
public:
    friend class SiTableInterface;
    ~SdtTable();    

    void AddService(ServiceId serviceId, uchar_t eitScheduleFlag, 
                    uchar_t eitPresentFollowingFlag, uint16_t runningStatus, uint16_t freeCaMode);
    void AddServiceDescriptor(ServiceId serviceId, std::string &data);

    SiTableKey GetKey() const;
    TableId GetTableId() const;

protected:
    bool CheckTableId(TableId tableId) const;
    bool CheckTsId(TsId tsid) const;
    size_t GetFixedSize() const;
    size_t GetVarSize() const;
    const Var1& GetVar1() const;
    Var2 GetVar2(TableId tableId) const;
    size_t MakeCodes1(TableId tableId, uchar_t *buffer, size_t bufferSize, size_t var1Size,
                      SectionNumber secNumber, SectionNumber lastSecNumber) const;    
    size_t MakeCodes2(Var2 &var2, uchar_t *buffer, size_t bufferSize,
                      size_t var2MaxSize, size_t var2Offset) const;  

private:
    SdtTable(TableId tableId, TsId transportStreamId, Version versionNumber, NetId originalNetworkId);

private:
    TableId  tableId;    
    TsId transportStreamId;
    Version  versionNumber;
    NetId originalNetworkId;

    VarHelper varHelper;
    SdtServices sdtServices;
};

#endif