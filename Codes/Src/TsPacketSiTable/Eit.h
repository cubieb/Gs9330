#ifndef _Eit_h_
#define _Eit_h_

#include "Include/Foundation/Time.h"
#include "Descriptor.h"       //Descriptor 
#include "TransportStream.h"  //TransportStream
#include "SiTableTemplate.h"

#pragma pack(push, 1)
struct event_information_section
{
    uchar_t table_id: 8;

    uint16_t section_syntax_indicator: 1;
    uint16_t reserved_future_use: 1;
    uint16_t reserved1: 2;
    uint16_t section_length: 12;

    uint16_t service_id: 16;

    uchar_t reserved2: 2;
    uchar_t version_number: 5;
    uchar_t current_next_indicator: 1;

    uchar_t section_number: 8;
    uchar_t last_section_number: 8;
    uint16_t transport_stream_id: 16;
    uint16_t original_network_id: 16;
    uchar_t  segment_last_section_number: 8;
    uchar_t  last_table_id: 8;
    /*  ... ... */
    uint32_t CRC_32: 32; 
};

struct event_information_section_detail
{
    //for (i=0;i<N;i++)
    //{
    uint16_t event_id:           16;       //uimsbf     --

    uint64_t start_time:         40;
    uint64_t duration:           24;

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
#define EitFixedFieldSize sizeof(event_information_section)
//MaxEitEventContentSize = 4078
#define MaxEitEventContentSize (MaxEitSectionLength - EitFixedFieldSize)
//sizeof(event_information_section_detail) = 12
#define MaxEitEventDescriptorSize (MaxEitEventContentSize - sizeof(event_information_section_detail))

/**********************class EitEvent**********************/
class EitEvent
{
public:
    EitEvent(EventId eventId, const char *startTime, 
             time_t duration, uint16_t runningStatus, uint16_t freeCaMode);
    ~EitEvent();
   
    void AddDescriptor(Descriptor *descriptor);

    size_t GetCodesSize() const;
    time_t GetDuration() const;
    EventId GetEventId() const;
    const std::string &GetStartTime() const;

    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const; 

private:
    EventId  eventId;  
    std::string  startTime;
    time_t   duration; 
    uint16_t runningStatus;
    uint16_t freeCaMode;

    Descriptors descriptors;
};

class CompareEitEventId: std::unary_function<EitEvent, bool>
{
public:
    CompareEitEventId(EventId eventId)
        : eventId(eventId)
    {}

    result_type operator()(const argument_type &eitEvent)
    {
        return (result_type)(eitEvent.GetEventId() == eventId);
    }

    result_type operator()(const argument_type *eitEvent)
    {
        return this->operator()(*eitEvent);
    }

private:
    uint16_t eventId;
};

class CompareEitEventTime: std::unary_function<EitEvent, bool>
{
public:
    CompareEitEventTime(time_t time)
        : time(time)
    {}

    result_type operator()(const argument_type &eitEvent)
    {
        time_t eventTime = ConvertStrToTime(eitEvent.GetStartTime().c_str()) + eitEvent.GetDuration();
        
        return eventTime < time;
    }

    result_type operator()(const argument_type *eitEvent)
    {
        return this->operator()(*eitEvent);
    }

private:
    time_t time;
};


/**********************class EitEvents**********************/
class EitEvents: public ContainerBase
{
public:
    EitEvents();
    ~EitEvents();
    
    void AddEvent(EitEvent *eitEvent);
    void AddEventDescriptor(uint16_t eventId, Descriptor *descriptor);    
    
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

    size_t GetCodesSize(size_t maxSize, size_t offset, 
                        uint_t maxEventNumberIn1Section, uint_t maxEventNumberInAllSection) const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize, size_t offset,
                     uint_t maxEventNumberIn1Section, uint_t maxEventNumberInAllSection) const;
    
    /* remove out-of-date event 
       Return Values:  
           true if events is same between befor and after.
           false if some out-of-date event was deleted.
     */
    bool RemoveOutOfDateEvent();

private:
    std::list<EitEvent *>::const_iterator Seek(size_t offset, uint_t maxEventNumberInAllSection) const;

private:
    std::list<EitEvent *> eitEvents;
};

template<typename EitEvents>
class EitEventsBinder: public IteratorBase 
{
public:
    EitEventsBinder(const EitEvents &eitEvents, uint_t maxEventNumberIn1Section, uint_t maxEventNumberInAllSection)
        : eitEvents(eitEvents),
          maxEventNumberIn1Section(maxEventNumberIn1Section),
          maxEventNumberInAllSection(maxEventNumberInAllSection)
    {
        const ContainerBase *container = &eitEvents;
        this->Adopt(container); 
    }

    size_t GetCodesSize(size_t maxSize, size_t offset) const
    {
#ifdef _DEBUG
        assert(this->GetContainer() != nullptr);
#endif
        return eitEvents.GetCodesSize(maxSize, offset, maxEventNumberIn1Section, maxEventNumberInAllSection);
    }

    size_t MakeCodes(uchar_t *buffer, size_t bufferSize, size_t offset) const
    {
#ifdef _DEBUG
        assert(this->GetContainer() != nullptr);
#endif
        return eitEvents.MakeCodes(buffer, bufferSize, offset, maxEventNumberIn1Section, maxEventNumberInAllSection);
    }

private:
    const EitEvents &eitEvents;
    uint_t maxEventNumberIn1Section;
    uint_t maxEventNumberInAllSection;
};

/**********************class EitTable**********************/
class EitTable: public SiTableTemplate<VarHelper, EitEventsBinder<EitEvents>>
{
public:
    friend class SiTableInterface;
    ~EitTable(); 

    void AddEvent(EventId eventId, const char *startTime, 
                  time_t duration, uint16_t runningStatus, uint16_t freeCaMode);
    void AddEventDescriptor(EventId eventId, std::string &data);
    
    SiTableKey GetKey() const;
    TableId GetTableId() const;
    void RefreshCatch();

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
    EitTable(TableId tableId, ServiceId serviceId, Version versionNumber, 
             TsId transportStreamId, NetId originalNetworkId);

private:    
    TableId tableId;
    ServiceId serviceId;
    Version versionNumber;
    TsId    transportStreamId;
    NetId   originalNetworkId;

    VarHelper varHelper;
    EitEvents eitEvents;
};

#endif