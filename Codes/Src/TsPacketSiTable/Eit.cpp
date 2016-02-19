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

size_t EitEvents::GetCodesSize(TableId tableId, size_t maxSize, size_t &offset) const
{
    size_t size = 0;
    size_t curOffset = 0;
    uint_t number = 0;
    for (const auto iter: eitEvents)
    {
        if (tableId == EitActualPfTableId || tableId == EitOtherPfTableId)
        {
            if (number++ == 2)
            {
                //we have packed 2 event in current section or previous section.
                break;
            }
        }

        if (curOffset < offset)
        {
            curOffset = curOffset + iter->GetCodesSize();
            continue;
        }
        assert(curOffset == offset);

        if (size + iter->GetCodesSize() > maxSize)
        {
            //at lest 1 EitEvent is counted.
            assert(size != 0);
            break;
        }

        size = size + iter->GetCodesSize();        
    }

    offset = offset + size;
    return size; 
}

size_t EitEvents::MakeCodes(TableId tableId, uchar_t *buffer, size_t bufferSize,
                            size_t offset) const
{
    //we assume EitActualPfTableId and EitOtherPfTableId always can be packed in single 
    //one section.
    assert(offset == 0 || (tableId == EitActualSchTableId || tableId == EitOtherSchTableId));

    uchar_t *ptr = buffer;  

    size_t curOffset = 0;
    uint_t number = 0;

    for (const auto iter: eitEvents)
    {
        if (tableId == EitActualPfTableId || tableId == EitOtherPfTableId)
        {
            if (number++ == 2)
            {
                //we have packed 2 event in current section or previous section.
                break; 
            }
        }

        if (curOffset < offset)
        {
            curOffset = curOffset + iter->GetCodesSize();
            continue;
        }
        assert(curOffset == offset); 

        if (ptr + iter->GetCodesSize() > buffer + bufferSize)
        {
            //at lest 1 EitEvent is counted.
            assert(ptr != buffer);
            break;
        }

        ptr = ptr + iter->MakeCodes(ptr, buffer + bufferSize - ptr);
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

size_t EitTable::GetCodesSize(TableId tableId, const TsIds &tsIds,
                              SectionNumber secIndex) const
{
    if (!CheckTableId(tableId))
    {
        return 0;
    }
    
#ifdef UseCatchOptimization
    CatchId catchId = catchIdHelper.GetCatchId(tableId, tsIds, secIndex);
    map<CatchId, size_t>::iterator catchIter = codeSizeCatches.find(catchId);
    if (catchIter != codeSizeCatches.end())
    {
        return catchIter->second;
    }
#endif
    
    TsIds::const_iterator iter;
    iter = find(tsIds.cbegin(), tsIds.cend(), transportStreamId);
    if (iter == tsIds.cend())
    {
#ifdef UseCatchOptimization
        codeSizeCatches.insert(make_pair(catchId, 0));
#endif
        return 0;
    }

    //check secIndex is valid.
    assert(secIndex < (SectionNumber)GetSecNumber(tableId, tsIds));

    size_t eventOffset = 0;
    for (SectionNumber i = 0; i < secIndex; ++i)
    {
        eitEvents.GetCodesSize(tableId, MaxEitEventContentSize, eventOffset);
    }

    size_t size = eitEvents.GetCodesSize(tableId, MaxEitEventContentSize, eventOffset)
                  + sizeof(event_information_section); 

#ifdef UseCatchOptimization
    codeSizeCatches.insert(make_pair(catchId, size));
#endif

    return size;
}

SiTableKey EitTable::GetKey() const
{
    SiTableKey key = (transportStreamId << 16) | serviceId;
    return key;
}

uint_t EitTable::GetSecNumber(TableId tableId, const TsIds &tsIds) const
{
    if (!CheckTableId(tableId))
    {
        return 0;
    }

#ifdef UseCatchOptimization
    CatchId catchId = catchIdHelper.GetCatchId(tableId, tsIds);
    map<CatchId, uint_t>::iterator catchIter = secNumberCatches.find(catchId);
    if (catchIter != secNumberCatches.end())
    {
        return catchIter->second;
    }
#endif

    TsIds::const_iterator iter;
    iter = find(tsIds.cbegin(), tsIds.cend(), transportStreamId);
    if (iter == tsIds.cend())
    {
#ifdef UseCatchOptimization
        secNumberCatches.insert(make_pair(catchId, 0));
#endif
        return 0;
    }

    uint_t secNumber = 1;
    size_t eventOffset = 0;
    eitEvents.GetCodesSize(tableId, MaxEitEventContentSize, eventOffset);

    size_t tsSize;
    for (tsSize = eitEvents.GetCodesSize(tableId, MaxEitEventContentSize, eventOffset);
         tsSize != 0;
         tsSize = eitEvents.GetCodesSize(tableId, MaxEitEventContentSize, eventOffset))
    {
        ++secNumber;
    }

#ifdef UseCatchOptimization
    secNumberCatches.insert(make_pair(catchId, secNumber));
#endif

    return secNumber;
}

TableId EitTable::GetTableId() const
{
    return tableId;
}

size_t EitTable::MakeCodes(TableId tableId, const TsIds &tsIds, 
						   uchar_t *buffer, size_t bufferSize,
                           SectionNumber secIndex) const
{
    uchar_t *ptr = buffer;
    size_t  size = GetCodesSize(tableId, tsIds, secIndex);
    assert(size <= bufferSize);
    if (size == 0)
        return 0;

    //check if secIndex is valid.
    assert(secIndex < (SectionNumber)GetSecNumber(tableId, tsIds));

#ifdef UseCatchOptimization
    CatchId catchId = catchIdHelper.GetCatchId(tableId, tsIds, secIndex);
    map<CatchId, uchar_t*>::iterator catchIter = codeCatches.find(catchId);
    if (catchIter != codeCatches.end())
    {
        memcpy(buffer, catchIter->second, size);
        return size;
    }
#endif

    size_t eventOffset = 0;
    SectionNumber secNumber = GetSecNumber(tableId, tsIds);
    for (SectionNumber i = 0; i < secIndex; ++i)
    {
        eitEvents.GetCodesSize(tableId, MaxEitEventContentSize, eventOffset);
    }

    ptr = ptr + Write8(ptr, tableId);
    WriteHelper<uint16_t> siHelper(ptr, ptr + 2);
    ptr = ptr + Write16(ptr, 0); 
    ptr = ptr + Write16(ptr, serviceId);    //service_id

    uchar_t currentNextIndicator = 1;
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator); //version_number
    ptr = ptr + Write8(ptr, secIndex);      //section_number
    ptr = ptr + Write8(ptr, secNumber - 1);  //last_section_number
    ptr = ptr + Write16(ptr, transportStreamId); //transport_stream_id
    ptr = ptr + Write16(ptr, originalNetworkId); //original_network_id
    ptr = ptr + Write8(ptr, secNumber - 1);  //segment_last_section_number
    ptr = ptr + Write8(ptr, tableId);       //????
    
    ptr = ptr + eitEvents.MakeCodes(tableId, ptr, MaxEitEventContentSize, eventOffset);
    
    siHelper.Write((EitSectionSyntaxIndicator << 15) | (Reserved1Bit << 14) | (Reserved2Bit << 12), ptr + 4); 
    ptr = ptr + Write32(ptr, Crc32::CalculateCrc(buffer, ptr - buffer));

#ifdef UseCatchOptimization
    uchar_t *codeCatch = new uchar_t[size];
    memcpy(codeCatch, buffer, size);

    codeCatches.insert(make_pair(catchId, codeCatch));
#endif

    assert(ptr - buffer == size);
    return size;
}

void EitTable::RefreshCatch()
{
    if (eitEvents.RemoveOutOfDateEvent())
    {
        return;
    }
    ClearCatch();
}

/* private function */
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

void EitTable::ClearCatch()
{
#ifdef UseCatchOptimization
    codeSizeCatches.clear();
    map<CatchId, uchar_t*>::iterator iter;
    for (iter = codeCatches.begin(); iter != codeCatches.end(); ++iter)
    {
        delete[] iter->second;
    }
    codeCatches.clear();
    secNumberCatches.clear();
#endif
}