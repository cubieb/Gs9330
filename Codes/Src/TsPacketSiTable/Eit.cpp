#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Deleter.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/PacketHelper.h"
#include "Include/Foundation/Converter.h"
#include "Include/Foundation/Crc32.h"
#include "Include/Foundation/Time.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Eit.h"
using namespace std;

EitTableInterface * EitTableInterface::CreateInstance(TableId tableId, ServiceId serviceId, Version versionNumber,
                                                      TsId transportStreamId, NetId originalNetworkId)
{
    return new EitTable(tableId, serviceId, versionNumber, transportStreamId, originalNetworkId);
}

/**********************public function**********************/
uint32_t ConvertDateToMjd(uint32_t year, uint32_t month, uint32_t day)
{
    uint32_t l = (month == 1 || month == 2) ? 1 : 0;
    return (14956 + day + (uint32_t)((year - l) * 365.25) + (uint32_t)((month + 1 + l * 12) * 30.6001));
}

void ConvertMjdToDate(uint32_t mjd, uint32_t &year, uint32_t &month, uint32_t &day)
{
     year  = (uint32_t)((mjd - 15078.2) / 365.25);
     month = (uint32_t)((mjd - 14956.1 - uint32_t(year * 365.25)) / 30.6001);
     day   = mjd - 14956 - uint32_t(year * 365.25) - uint32_t(month * 30.6001);
     uint_t k = (month == 14 || month == 15) ? 1 : 0;
     year = year + k;
     month = month - 1 - k * 12;
}

/**********************class EitEvent**********************/
EitEvent::EitEvent(EventId eventId, const char *startTime, 
                   time_t duration, uint16_t runningStatus, uint16_t freeCaMode)
    : eventId(eventId), startTime(startTime), 
      duration(duration), runningStatus(runningStatus), freeCaMode(freeCaMode)
{
}

EitEvent::~EitEvent()
{
}

void EitEvent::AddDescriptor(Descriptor *descriptor)
{
    descriptors.AddDescriptor(descriptor);
}

size_t EitEvent::GetCodesSize() const
{
    return (descriptors.GetCodesSize() + 10);
}

time_t EitEvent::GetDuration() const
{
    return duration;
}

EventId EitEvent::GetEventId() const
{
    return eventId;
}

const std::string &EitEvent::GetStartTime() const
{
    return startTime;
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

    ptr = ptr + descriptors.MakeCodes(ptr, bufferSize - 3, (runningStatus << 1) | freeCaMode);
    
    assert(ptr - buffer == size);
    return (ptr - buffer);
}

/**********************class EitEvents**********************/
EitEvents::EitEvents()
{
}

EitEvents::~EitEvents()
{
    for_each(eitEvents.begin(), eitEvents.end(), ScalarDeleter());
}

size_t EitEvents::GetCodesSize(TableId tableId) const
{
    /* there is no reserved_future_use and xxx_xxx__length fields, so we
       have to impliment GetCodesSize() and MakeCodes() myself.
     */
    size_t size = 0;
    uint_t number = 0;
    for (const auto iter: eitEvents)
    {
        if (tableId == EitActualPfTableId || tableId == EitOtherPfTableId)
        {
            if (number++ == 2)
                break;
        }

        size = size + iter->GetCodesSize();
    }

    return size; 
}

size_t EitEvents::MakeCodes(TableId tableId, uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;  
    size_t size = GetCodesSize(tableId);
    assert(size <= bufferSize);

    uint_t number = 0;
    for (const auto iter: eitEvents)
    {
        if (tableId == EitActualPfTableId || tableId == EitOtherPfTableId)
        {
            if (number++ == 2)
                break;
        }

        ptr = ptr + iter->MakeCodes(ptr, bufferSize - (ptr - buffer));
    }
    assert(ptr - buffer == size);
    return size;
}

void EitEvents::AddEvent(EitEvent *eitEvent)
{
    eitEvents.push_back(eitEvent);
}

void EitEvents::AddEventDescriptor(uint16_t eventId, Descriptor *descriptor)
{
    list<EitEvent *>::iterator iter;
    iter = find_if(eitEvents.begin(), eitEvents.end(), CompareEitEventId(eventId));
    (*iter)->AddDescriptor(descriptor);
}

void EitEvents::RemoveOutOfDateEvent()
{
    time_t curTime = time(nullptr);
    eitEvents.remove_if(CompareEitEventTime(curTime));
}

/**********************class EitTable**********************/
EitTable::EitTable(TableId tableId, ServiceId serviceId, Version versionNumber, TsId transportStreamId, NetId originalNetworkId)
    : tableId(tableId), serviceId(serviceId), sectionNumber(0), lastSectionNumber(0), 
      transportStreamId(transportStreamId), originalNetworkId(originalNetworkId), versionNumber(versionNumber)
{
}

EitTable::~EitTable()
{
}

void EitTable::AddEvent(EventId eventId, const char *startTime, 
                time_t duration, uint16_t  runningStatus, uint16_t freeCaMode)
{
    EitEvent *eitEvent = new EitEvent(eventId, startTime, duration, runningStatus, freeCaMode);
    eitEvents.AddEvent(eitEvent);
}

void EitTable::AddEventDescriptor(EventId eventId, std::string &data)
{
    Descriptor* descriptor = CreateDescriptor(data);
    if (descriptor == nullptr)
    {
        errstrm << "we do not support descriptor whose tag = " 
            << hex << data[0] << data[1] << endl;
        return;
    }

    eitEvents.AddEventDescriptor(eventId, descriptor);
}

size_t EitTable::GetCodesSize(TableId tableId, const std::list<TsId>& tsIds) const
{
    eitEvents.RemoveOutOfDateEvent();
    return GetCodesSizeImp(tableId, tsIds);
}

uint16_t EitTable::GetKey() const
{
    return serviceId;
}

TableId EitTable::GetTableId() const
{
    return tableId;
}

size_t EitTable::MakeCodes(TableId tableId, const std::list<TsId>& tsIds, 
						   uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    uint16_t ui16Value; 
    size_t  size = GetCodesSizeImp(tableId, tsIds);

    if (size == 0)
        return 0;

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
    
    ptr = ptr + eitEvents.MakeCodes(tableId, ptr, bufferSize - (ptr - buffer));

    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.CalculateCrc(buffer, ptr - buffer));
    assert(ptr - buffer == size);
    return size;
}

/* private function */
size_t EitTable::GetCodesSizeImp(TableId tableId, const std::list<TsId>& tsIds) const
{
    if (this->tableId == EitActualSchTableId)
    {
        if (tableId != EitActualPfTableId && tableId != EitActualSchTableId)
        {
            return 0;
        }
    }
    else
    {
        if (tableId != EitOtherPfTableId && tableId != EitOtherSchTableId)
        {
            return 0;
        }
    }

    std::list<TsId>::const_iterator iter;
    iter = find(tsIds.cbegin(), tsIds.cend(), transportStreamId);
    if (iter == tsIds.cend())
        return 0;

    size_t size = eitEvents.GetCodesSize(tableId);
    return size + sizeof(event_information_section);
}