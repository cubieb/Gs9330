#ifndef _Eit_h_
#define _Eit_h_

#include <time.h>
#include "Component.h"

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
#pragma pack(pop)

/**********************class EitEvent**********************/
class EitEvent: public Component
{
    friend class OutdatedEitEvents;
public:
    EitEvent(uint16_t eventId, const char *startTime, 
        time_t duration, uint16_t runningStatus, uint16_t freeCaMode);
    
    uint16_t GetEventId() const;

    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    void AddServiceDescriptor0x48(uchar_t serviceType, uchar_t *providerName, uchar_t *serviceName);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;   

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uint16_t eventId; 
    std::string  startTime;
    time_t   duration; 
    uint16_t runningStatus; 
    uint16_t freeCaMode;
    std::shared_ptr<Descriptors> descriptors;
};

class EqualEitEvents: std::unary_function<const std::shared_ptr<Component>&, bool>
{
public:
    EqualEitEvents(uint16_t eventId)
        : eventId(eventId)
    {}

    result_type operator()(argument_type component)
    {
        EitEvent& event = dynamic_cast<EitEvent&>(*component);
        return (result_type)(event.GetEventId() == eventId);
    }

private:
    uint16_t eventId;
};

class OutdatedEitEvents: std::unary_function<const std::shared_ptr<Component>&, bool>
{
public:
    OutdatedEitEvents(time_t rawTime)
        : rawTime(rawTime)
    {}

    result_type operator()(argument_type component)
    {
        EitEvent& event = dynamic_cast<EitEvent&>(*component);
        time_t eventTime = ConvertStrToTime(event.startTime.c_str());
        
        return eventTime < rawTime;
    }

private:
    time_t rawTime;
};


/**********************class EitEvents**********************/
class EitEvents: public Components
{
public:
    typedef Components MyBase;
    friend class Eit;

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    void AddEvent(uint16_t eventId, const char *startTime, 
        time_t duration, uint16_t  runningStatus, uint16_t freeCaMode);
    void AddEventDescriptor(uint16_t eventId, uchar_t tag, uchar_t* data, size_t dataSize);

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;
};

/**********************class Eit**********************/
class Eit: public Section
{
public:
    /* SI PID definition: <iso13818-1.pdf>, 5.1.3 Coding of PID and table_id fields */
    enum: uint16_t {Pid = 0x0012};
    Eit(const char *key);
    ~Eit() {}
    
    uint16_t GetPid()  const; 

    void SetTableId(uchar_t data);
    uchar_t GetTableId() const;

    void SetNetworkId(uint16_t data);
    uint16_t GetNetworkId() const;

    void SetServiceId(uint16_t data);
    void SetVersionNumber(uchar_t data);
    void SetSectionNumber(uchar_t data);
    void SetLastSectionNumber(uchar_t data);
    void SetTsId(uint16_t data);
    void SetOnId(uint16_t data);

    void AddEvent(uint16_t eventId, const char *startTime, time_t duration,
                  uint16_t  runningStatus, uint16_t freeCaMode);
    void AddEventDescriptor(uint16_t eventId, uchar_t tag, uchar_t* data, size_t dataSize);
    
    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    size_t GetCodesSizeExt();
    size_t MakeCodesExt(uchar_t *buffer, size_t bufferSize);

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uchar_t  tableId;
    //protocol do not define network Id for this section, we use this networkId to
    //determine whether the current section should be send to a sub-network
    uint16_t networkId;   
    uint16_t serviceId;
    uchar_t  versionNumber;
    uchar_t  sectionNumber;
    uchar_t  lastSectionNumber;
    uint16_t transportStreamId;
    uint16_t originalNetworkId;
    //uchar_t  tableId;

    std::string startTime;
    std::chrono::seconds duration;

    std::shared_ptr<EitEvents> events;
};

#endif