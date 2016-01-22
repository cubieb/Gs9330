#ifndef _Eit_h_
#define _Eit_h_

#include "Include/Foundation/Time.h"
#include "Descriptor.h"       //Descriptor 
#include "TransportStream.h"  //TransportStream

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
class EitEvents
{
public:
    EitEvents();
    ~EitEvents();

    size_t GetCodesSize(TableId tableId) const;
    size_t MakeCodes(TableId tableId, uchar_t *buffer, size_t bufferSize) const;

    void AddEvent(EitEvent *eitEvent);
    void AddEventDescriptor(uint16_t eventId, Descriptor *descriptor);

    /* remove out-of-date event */
    void RemoveOutOfDateEvent();

private:
    std::list<EitEvent *> eitEvents;
};

/**********************class EitTable**********************/
class EitTable: public EitTableInterface
{
public:
    EitTable(TableId tableId, ServiceId serviceId, Version versionNumber, TsId transportStreamId, NetId originalNetworkId);
    ~EitTable(); 

    void AddEvent(EventId eventId, const char *startTime, 
                  time_t duration, uint16_t runningStatus, uint16_t freeCaMode);
    void AddEventDescriptor(EventId eventId, std::string &data);

    size_t GetCodesSize(TableId tableId, const std::list<TsId>& tsIds) const;

    uint16_t GetKey() const;
    TableId GetTableId() const;
    size_t MakeCodes(TableId tableId, const std::list<TsId>& tsIds, 
                     uchar_t *buffer, size_t bufferSize) const;

private:
    size_t GetCodesSizeImp(TableId tableId, const std::list<TsId>& tsIds) const;

private:    
    TableId tableId;
    ServiceId serviceId;
    Version versionNumber;    
    SectionNumber sectionNumber;
    SectionNumber lastSectionNumber;
    TsId    transportStreamId;
    NetId originalNetworkId;
    mutable EitEvents eitEvents;
};

#pragma pack(pop)
#endif