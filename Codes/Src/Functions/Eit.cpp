#include "SystemInclude.h"
#include <regex>
#include "Common.h"
#include "Debug.h"

#include "Crc32.h"
#include "Descriptor.h"
#include "Eit.h"
using namespace std;

uint32_t ConvertDateToMjd(uint32_t year, uint32_t month, uint32_t day)
{
    uint32_t l = (month == 1 || month == 2) ? 1 : 0;
    return (14956 + day + (uint32_t)((year - l) * 365.25) + (uint32_t)((month + 1 + l * 12) * 30.6001));
}

uint64_t ConvertDateToMjd(uint64_t year, uint64_t month, uint64_t day)
{
    uint64_t l = (month == 1 || month == 2) ? 1 : 0;
    return (14956 + day + (uint64_t)((year - l) * 365.25) + (uint64_t)((month + 1 + l * 12) * 30.6001));
}

/**********************class EitEvent**********************/
EitEvent::EitEvent(uint16_t eventId, const char *startTime, 
        time_t duration, uint16_t runningStatus, uint16_t freeCaMode)
    : eventId(eventId), startTime(startTime), duration(duration), 
      runningStatus(runningStatus), freeCaMode(freeCaMode)
{
    descriptors.reset(new Descriptors);
}

uint16_t EitEvent::GetEventId() const
{
    return eventId;
}

void EitEvent::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    descriptors->AddDescriptor(tag, data, dataSize);
}

size_t EitEvent::GetCodesSize() const
{
    return (descriptors->GetCodesSize() + 10);
}

size_t EitEvent::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    size_t size = GetCodesSize();
    assert(size <= bufferSize);
    
    ptr = ptr + Write16(ptr, eventId);

    tm localTime, gmtTime;  
    std::istringstream ss(this->startTime);
    ss >> std::get_time(&localTime, "%Y-%m-%d %H:%M:%S");
    ConvertUtcToGmt(localTime, gmtTime);

    uint32_t year, month, day, hour, minute, second;
    year = gmtTime.tm_year;
    month = gmtTime.tm_mon + 1;
    day = gmtTime.tm_mday;
    hour = gmtTime.tm_hour;
    minute = gmtTime.tm_min;
    second = gmtTime.tm_sec;

    uint64_t startDate = ConvertDateToMjd(year, month, day);
    uint64_t startTime = ((hour / 10) << 20) | ((hour % 10) << 16) 
                | ((minute / 10) << 12) | ((minute % 10) << 8) 
                | ((second / 10) << 4) | (second % 10);

    chrono::seconds total(this->duration);
    chrono::hours hours = chrono::duration_cast<chrono::hours>(total);
    chrono::minutes minutes = chrono::duration_cast<chrono::minutes>(total - hours);
    chrono::seconds seconds = chrono::duration_cast<chrono::seconds>(total - hours - minutes);

    uint64_t duration = ((hours.count() / 10) << 20) | ((hours.count() % 10) << 16) 
        | ((minutes.count() / 10) << 12) | ((minutes.count() % 10) << 8) 
        | ((seconds.count() / 10) << 4) | (seconds.count() % 10);
    ptr = ptr + Write64(ptr, (((startDate << 24) | (startTime)) << 24) | (duration & 0xFFFFFF));

    ptr = ptr + descriptors->MakeCodes(ptr, bufferSize - 3, (runningStatus << 1) | freeCaMode);
    
    assert(ptr - buffer == size);
    return (ptr - buffer);
}

/* the following function is provided just for debug */
void EitEvent::Put(std::ostream& os) const
{
    os << "event_id = " << (uint_t)eventId
        << ", start_time = " << startTime
        << ", duration = " << duration
        << ", running_status = " << (uint_t)runningStatus  
        << ", free_CA_mode = " << (uint_t)freeCaMode  
        << endl;

    os << *descriptors;
}

/**********************class EitEvents**********************/
size_t EitEvents::GetCodesSize() const
{    
    /* there is no reserved_future_use and xxx_xxx__length fields, so we
       have to impliment GetCodesSize() and MakeCodes() myself.
     */
    size_t size = 0;
    for (const auto iter: components)
    {
        size = size + iter->GetCodesSize();
    }
        
    return size; 
}

size_t EitEvents::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;  
    size_t size = GetCodesSize();
    assert(size <= bufferSize);

    for (const auto iter: components)
    {
        ptr = ptr + iter->MakeCodes(ptr, bufferSize - (ptr - buffer));
    }
    assert(ptr - buffer == size);
    return size;
}

void EitEvents::AddEvent(uint16_t eventId, const char *startTime, 
    time_t duration, uint16_t  runningStatus, uint16_t freeCaMode)
{
    auto event = make_shared<EitEvent>(eventId, startTime,
        duration, runningStatus, freeCaMode);
    AddComponent(event);
}

void EitEvents::AddEvent(std::shared_ptr<EitEvent> event)
{
    AddComponent(event);
}

void EitEvents::AddEventDescriptor(uint16_t eventId, uchar_t tag, uchar_t* data, size_t dataSize)
{
    auto iter = find_if(components.begin(), components.end(), EqualEitEvents(eventId));
    EitEvent& event = dynamic_cast<EitEvent&>(**iter);
    event.AddDescriptor(tag, data, dataSize);
}

void EitEvents::RemoveIf(time_t time)
{
    components.remove_if(OutdatedEitEvents(time));
}

void EitEvents::Clear()
{
    components.clear();
}

shared_ptr<EitEvent> EitEvents::GetFirstEitEvent()
{
    if (components.empty())
        return nullptr;

    return dynamic_pointer_cast<EitEvent>(components.front());
}

shared_ptr<EitEvent> EitEvents::GetSecondEitEvent()
{
    if (components.size() < 2)
        return nullptr;

    auto iter = components.begin();
    ++iter;

    return dynamic_pointer_cast<EitEvent>(*iter);
}

/* the following function is provided just for debug */
void EitEvents::Put(std::ostream& os) const
{
    MyBase::Put(os);
}

/**********************class Eit**********************/
Eit::Eit(const char *key)
    : Section(key), tableId(0), networkId(0), serviceId(0), versionNumber(0), 
            sectionNumber(0), lastSectionNumber(0), 
            transportStreamId(0), originalNetworkId(0)
{
    events.reset(new EitEvents);
}

uint16_t Eit::GetPid()  const
{
    return Pid;
}

void Eit::SetTableId(uchar_t data)
{
    /* Table 2: Allocation of table_id values */
    assert(data == 0x4E || data == 0x4F || data == 0x50 || data == 0x60);
    tableId = data;
}

uchar_t Eit::GetTableId() const
{
    return tableId;
}

void Eit::SetNetworkId(uint16_t data)
{
    networkId = data;
}

uint16_t Eit::GetNetworkId() const
{
    return networkId;
}

void Eit::SetServiceId(uint16_t data)
{
    serviceId = data;
}

void Eit::SetVersionNumber(uchar_t data)
{
    versionNumber = data;
}

void Eit::SetSectionNumber(uchar_t data)
{
    sectionNumber = data;
}

void Eit::SetLastSectionNumber(uchar_t data)
{
    lastSectionNumber = data;
}

void Eit::SetTsId(uint16_t data)
{
    transportStreamId = data;
}

void Eit::SetOnId(uint16_t data)
{
    originalNetworkId = data;
}

void Eit::AddEvent(uint16_t eventId, const char *startTime, time_t duration,
              uint16_t runningStatus, uint16_t freeCaMode)
{
    events->AddEvent(eventId, startTime, duration, runningStatus, freeCaMode);
}

void Eit::AddEventDescriptor(uint16_t eventId, uchar_t tag, uchar_t* data, size_t dataSize)
{
    events->AddEventDescriptor(eventId, tag, data, dataSize);
}

size_t Eit::GetCodesSize() const
{
    size_t size = events->GetCodesSize();
    return size + sizeof(event_information_section);
}

size_t Eit::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{  
    uchar_t *ptr = buffer;
    uint16_t ui16Value; 
    size_t  size = GetCodesSize();

    assert(size <= bufferSize && size <= (MaxEitSectionLength - 3));
    ptr = ptr + Write8(ptr, tableId);
    ui16Value = (EitSectionSyntaxIndicator << 15) | (Reserved1Bit << 14) | (Reserved2Bit << 12) | (size - 3);
    ptr = ptr + Write16(ptr, ui16Value);    //section_length
    ptr = ptr + Write16(ptr, serviceId);    //service_id

    uchar_t currentNextIndicator = 1;
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator); //version_number
    ptr = ptr + Write8(ptr, sectionNumber);      //section_number
    ptr = ptr + Write8(ptr, lastSectionNumber);  //last_section_number
    ptr = ptr + Write16(ptr, transportStreamId); //transport_stream_id
    ptr = ptr + Write16(ptr, originalNetworkId); //original_network_id
    ptr = ptr + Write8(ptr, lastSectionNumber);  //segment_last_section_number
    ptr = ptr + Write8(ptr, tableId);       //????
    
    ptr = ptr + events->MakeCodes(ptr, bufferSize - (ptr - buffer));

    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.CalculateCrc(buffer, ptr - buffer));
    assert(ptr - buffer == size);
    return size;
}

void Eit::PropagateSection()
{
    /* 1 Create sub-section */
    if (subPresentSection == nullptr)
    {
        subPresentSection = make_shared<Eit>(key.c_str()); 
        CloneTo(*subPresentSection);
        subPresentSection->SetSectionNumber(0);
        subPresentSection->SetLastSectionNumber(1);
    }
    
    if (subFollwingtSection == nullptr)
    {
        subFollwingtSection = make_shared<Eit>(key.c_str()); 
        CloneTo(*subFollwingtSection);
        subFollwingtSection->SetSectionNumber(1);
        subFollwingtSection->SetLastSectionNumber(1);
    }

    /* 2 remove out-of-date event */
    events->RemoveIf(time(nullptr));
    subPresentSection->events->Clear();
    subFollwingtSection->events->Clear();

    shared_ptr<EitEvent> eitEvent;

    /* 3 add event to sub-section */
    eitEvent = events->GetFirstEitEvent();
    if (eitEvent != nullptr)
        subPresentSection->events->AddEvent(eitEvent);

    eitEvent = events->GetSecondEitEvent();
    if (eitEvent != nullptr)
        subFollwingtSection->events->AddEvent(eitEvent);
}

void Eit::CloneTo(Eit& eit)
{        
    uchar_t  tableId;
    if (this->tableId == 0x50)
        tableId = 0x4E;
    else if (this->tableId == 0x60)
        tableId = 0x4F;
    else
        assert(false);

    eit.SetTableId(tableId);
    eit.SetTsId(transportStreamId);
    eit.SetOnId(originalNetworkId);
    eit.SetVersionNumber(versionNumber);
    eit.SetServiceId(serviceId);
    eit.SetNetworkId(networkId);
}

std::shared_ptr<Eit> Eit::GetSubPresentSection()
{
    return subPresentSection;
}

std::shared_ptr<Eit> Eit::GetSubFollwingtSection()
{
    return subFollwingtSection;
}

void Eit::Put(std::ostream& os) const
{
    ios::fmtflags flags = cout.flags( );
    os  << "table_id = " << showbase << hex <<(uint_t)tableId
        << ", section_length = " << dec << (uint_t)GetCodesSize() - 3
        << ", service_id = " << dec << serviceId
        << ", version_number = " << hex << (uint_t)versionNumber 
        << ", section_number = " << hex << (uint_t)sectionNumber 
        << ", last_section_number = " << hex << (uint_t)lastSectionNumber 
        << ", transport_stream_id = " << (uint_t)transportStreamId
        << ", original_network_id = " << (uint_t)originalNetworkId
        << endl ;
    cout.flags(flags);

    os << *events;
}