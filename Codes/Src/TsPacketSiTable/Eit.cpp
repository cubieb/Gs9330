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

SiTableInterface * SiTableInterface::CreateEitInstance(TableId tableId, ServiceId serviceId, Version versionNumber,
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
    size_t size = descriptors.GetCodesSize() + sizeof(event_information_section_detail);
    assert(size <= MaxEitEventContentSize);
    return size;
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
    ConvertStrToTm(this->startTime.c_str(), localTime);
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

    WriteHelper<uint16_t> desHelper(ptr, ptr + 2);
    //fill "reserved_future_use + network_descriptors_length" to 0 temporarily.
    ptr = ptr + Write16(ptr, 0); 
    ptr = ptr + descriptors.MakeCodes(ptr, bufferSize - (ptr - buffer));
    desHelper.Write((runningStatus << 13) | (freeCaMode << 12), ptr); 
    
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

/* Input and Outpu  decision table.
TableId     offset                                   return value
-------     ---------------------------------        -------------------------
0x4e, 0x4f  0                                        size of "0 ~ (MaxEventNumberIn1EitPfTable - 1)"
0x4e, 0x4f  0 + (MaxEventNumberIn1EitPfTable - 1)    size of "(MaxEventNumberIn1EitPfTable-1) ~ (MaxEventNumberIn1EitPfTable*2-1)"
0x4e, 0x4f  0 + (MaxEventNumberIn1EitPfTable - 1)    0
...
...
 */
size_t EitEvents::GetCodesSize(TableId tableId, size_t maxSize, size_t offset) const
{
    size_t size = 0;
    uint_t number = 0;
    
    list<EitEvent *>::const_iterator iter; 
    for (iter = Seek(offset, GetMaxEventNumber(tableId)); iter != eitEvents.end(); ++iter)
    {
        if (size + (*iter)->GetCodesSize() > maxSize)
        {
            //at lest 1 EitEvent is counted.
            assert(size != 0);
            break;
        }
        size = size + (*iter)->GetCodesSize();  

        if (tableId == EitActualPfTableId || tableId == EitOtherPfTableId)
        {
            if (++number == MaxEventNumberIn1EitPfTable)
            {
                //we have packed MaxEventNumberIn1EitPfTable event in current section or previous section.
                break;
            }
        }
    }

    return size; 
}

size_t EitEvents::MakeCodes(TableId tableId, uchar_t *buffer, size_t bufferSize,
                            size_t offset) const
{
    uchar_t *ptr = buffer;  
    uint_t number = 0;

    list<EitEvent *>::const_iterator iter;
    for (iter = Seek(offset, GetMaxEventNumber(tableId)); iter != eitEvents.end(); ++iter)
    {
        if (ptr + (*iter)->GetCodesSize() > buffer + bufferSize)
        {
            //at lest 1 EitEvent is counted.
            assert(ptr != buffer);
            break;
        }
        ptr = ptr + (*iter)->MakeCodes(ptr, buffer + bufferSize - ptr);         

        if (tableId == EitActualPfTableId || tableId == EitOtherPfTableId)
        {
            if (++number == MaxEventNumberIn1EitPfTable)
            {
                //we have packed MaxEventNumberIn1EitPfTable event in current section or previous section.
                break;
            }
        }
    }

    return (ptr - buffer);
}
   
bool EitEvents::RemoveOutOfDateEvent()
{
    time_t curTime = time(nullptr); 
    list<EitEvent *>::iterator iter, start, end;

    start = eitEvents.begin();
    if (start == eitEvents.end())
    {
        return true;
    }
    time_t eventTime = ConvertStrToTime((*start)->GetStartTime().c_str()) + (*start)->GetDuration();
    if (eventTime > curTime)
    {
        return true;
    }

    for (end = eitEvents.begin(); end != eitEvents.end(); ++end)
    {
        eventTime = ConvertStrToTime((*end)->GetStartTime().c_str()) + (*end)->GetDuration();
        if (eventTime > curTime)
        {
            ++end;
            break;
        }
    }

    for (iter = start; iter != end; ++iter)
    {
        delete *iter;
    }

    eitEvents.erase(start, end);
    return false;
}

/* private function */
uint_t EitEvents::GetMaxEventNumber(TableId tableId) const
{
    if (tableId == EitActualPfTableId || tableId == EitOtherPfTableId)
        return MaxEventNumberInAllEitPfTable;

    return UINT_MAX;
}

list<EitEvent *>::const_iterator EitEvents::Seek(size_t offset, uint_t maxEventNumber) const
{
    size_t curOffset = 0;

    list<EitEvent *>::const_iterator iter;
    for (iter = eitEvents.cbegin(); iter != eitEvents.cend(); ++iter, --maxEventNumber)
    {   
        if (maxEventNumber == 0)
        {
            return eitEvents.cend();
        }

        if (curOffset >= offset)
            break;

        curOffset = curOffset + (*iter)->GetCodesSize();
    }
    assert(curOffset == offset);

    return iter;
}

/**********************class EitTable**********************/
EitTable::EitTable(TableId tableId, ServiceId serviceId, Version versionNumber, 
                   TsId transportStreamId, NetId originalNetworkId)
    : tableId(tableId), serviceId(serviceId), versionNumber(versionNumber),
      transportStreamId(transportStreamId), originalNetworkId(originalNetworkId)
{
}

EitTable::~EitTable()
{    
    ClearCatch();
}

void EitTable::AddEvent(EventId eventId, const char *startTime, 
                time_t duration, uint16_t  runningStatus, uint16_t freeCaMode)
{
    EitEvent *eitEvent = new EitEvent(eventId, startTime, duration, runningStatus, freeCaMode);
    var2.AddEvent(eitEvent);
    ClearCatch();
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

    var2.AddEventDescriptor(eventId, descriptor);
    ClearCatch();
}


SiTableKey EitTable::GetKey() const
{
    SiTableKey key = (transportStreamId << 16) | serviceId;
    return key;
}

TableId EitTable::GetTableId() const
{
    return tableId;
}

void EitTable::RefreshCatch()
{
    if (var2.RemoveOutOfDateEvent())
    {
        return;
    }
    ClearCatch();
}

/* protected function */
bool EitTable::CheckTableId(TableId tableId) const
{
    if (this->tableId == EitActualSchTableId)
    {
        if (tableId != EitActualPfTableId && tableId != EitActualSchTableId)
        {
            return false;
        }
    }
    else
    {
        if (tableId != EitOtherPfTableId && tableId != EitOtherSchTableId)
        {
            return false;
        }
    }

    return true;
}

bool EitTable::CheckTsId(TsId tsid) const
{
    return (tsid == this->transportStreamId);
}

size_t EitTable::MakeCodes1(TableId tableId, uchar_t *buffer, size_t bufferSize, size_t var1Size,
                            SectionNumber secNumber, SectionNumber lastSecNumber) const
{
    uchar_t *ptr = buffer;
    
    //section_syntax_indicator:1 + reserved_future_use1:1 + reserved1:2 + section_length:12
    TableSize pseudoSize = 0;
    ptr = ptr + WriteBuffer(ptr, tableId);
    ptr = ptr + WriteBuffer(ptr, pseudoSize); 
    ptr = ptr + WriteBuffer(ptr, serviceId); //service_id

    uchar_t currentNextIndicator = 1;
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator); //version_number
    /* The section_number shall be incremented by 1 with each additional section with the same
       table_id, service_id, transport_stream_id, and original_network_id. In this case, the sub_table may be 
       structured as a number of segments.
     */
    ptr = ptr + Write8(ptr, secNumber);      //section_number
    ptr = ptr + Write8(ptr, lastSecNumber);  //last_section_number
    ptr = ptr + Write16(ptr, transportStreamId); //transport_stream_id
    ptr = ptr + Write16(ptr, originalNetworkId); //original_network_id
    ptr = ptr + Write8(ptr, lastSecNumber);  //segment_last_section_number
    ptr = ptr + Write8(ptr, tableId);       //????

    assert(ptr <= buffer + bufferSize);
    return (ptr - buffer);
}

size_t EitTable::MakeCodes2(TableId tableId, uchar_t *buffer, size_t bufferSize,
                            size_t var2MaxSize, size_t var2Offset) const
{
    uchar_t *ptr = buffer;

    ptr = ptr + var2.MakeCodes(tableId, ptr, var2MaxSize, var2Offset);

    assert(ptr <= buffer + bufferSize);
    return (ptr - buffer);
}

