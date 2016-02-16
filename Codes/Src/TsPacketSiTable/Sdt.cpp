#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Deleter.h"
#include "Include/Foundation/Debug.h"
#include "Include/Foundation/PacketHelper.h"
#include "Include/Foundation/Converter.h"
#include "Include/Foundation/Crc32.h"

/* TsPacketSiTable */
#include "Include/TsPacketSiTable/SiTableInterface.h"
#include "Sdt.h"
using namespace std;

SdtTableInterface * SdtTableInterface::CreateInstance(TableId tableId, TsId transportStreamId, 
                                                      Version versionNumber, NetId originalNetworkId)
{
    if (tableId != SdtActualTableId && tableId != SdtOtherTableId)
        return nullptr;

    return new SdtTable(tableId, transportStreamId, versionNumber, originalNetworkId);
}

/**********************class SdtService**********************/
SdtService::SdtService(ServiceId serviceId, uchar_t eitScheduleFlag, uchar_t eitPresentFollowingFlag,
           uint16_t runningStatus, uint16_t freeCaMode)
    : serviceId(serviceId), eitScheduleFlag(eitScheduleFlag), eitPresentFollowingFlag(eitPresentFollowingFlag), 
      runningStatus(runningStatus), freeCaMode(freeCaMode)
{
}

SdtService::~SdtService()
{
}

void SdtService::AddDescriptor(Descriptor *descriptor)
{
    descriptors.AddDescriptor(descriptor);
}

size_t SdtService::GetCodesSize() const
{
    size_t size = descriptors.GetCodesSize() + sizeof(service_description_section_detail);
    assert(size <= MaxSdtServiceContentSize);
    return size;
}

ServiceId SdtService::GetServiceId() const
{
    return serviceId;
}

size_t SdtService::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    size_t size = GetCodesSize();
    assert(size <= bufferSize);
    
    ptr = ptr + Write16(ptr, serviceId);
    ptr = ptr + Write8(ptr, (Reserved6Bit << 2) | (eitScheduleFlag << 1) | eitPresentFollowingFlag);

    WriteHelper<uint16_t> desHelper(ptr, ptr + 2);
    //fill "reserved_future_use + network_descriptors_length" to 0 temporarily.
    ptr = ptr + Write16(ptr, 0);  
    ptr = ptr + descriptors.MakeCodes(ptr, bufferSize - (ptr - buffer));
    desHelper.Write((runningStatus << 13) | (freeCaMode << 12), ptr); 

    assert(ptr - buffer == size);
    return (ptr - buffer);
}

/**********************class SdtServices**********************/
/* public function */
SdtServices::SdtServices()
{
}

SdtServices::~SdtServices()
{
    for_each(sdtServices.begin(), sdtServices.end(), ScalarDeleter());
}

void SdtServices::AddSdtService(SdtService* service)
{
    sdtServices.push_back(service);
}

void SdtServices::AddServiceDescriptor(ServiceId serviceId, Descriptor *descriptor)
{
    list<SdtService *>::iterator iter;
    iter = find_if(sdtServices.begin(), sdtServices.end(), CompareSdtServiceId(serviceId));
    (*iter)->AddDescriptor(descriptor);
}

size_t SdtServices::GetCodesSize(size_t maxSize, size_t &offset) const
{    
    size_t size = 0;
    size_t curOffset = 0;
    for (const auto iter: sdtServices)
    {
        if (curOffset < offset)
        {
            curOffset = curOffset + iter->GetCodesSize();
            continue;
        }
        assert(curOffset == offset);        

        if (size + iter->GetCodesSize() > maxSize)
        {
            //at lest 1 SdtService is counted.
            assert(size != 0);
            break;
        }

        size = size + iter->GetCodesSize();
    }

    offset = offset + size;
    return size; 
}

size_t SdtServices::MakeCodes(uchar_t *buffer, size_t bufferSize, size_t offset) const
{
    uchar_t *ptr = buffer;  

    size_t curOffset = 0;
    for (const auto iter: sdtServices)
    {
        if (curOffset < offset)
        {
            curOffset = curOffset + iter->GetCodesSize();
            continue;
        }
        assert(curOffset == offset);
        
        if (ptr + iter->GetCodesSize() > buffer + bufferSize)
        {
            //at lest 1 SdtService is counted.
            assert(ptr != buffer);
            break;
        }

        ptr = ptr + iter->MakeCodes(ptr, buffer + bufferSize - ptr);
    }

    return (ptr - buffer);
}

/**********************class SdtTable**********************/
/* public function */
SdtTable::SdtTable(TableId tableId, TsId transportStreamId, Version versionNumber, NetId originalNetworkId)
    : tableId(tableId), transportStreamId(transportStreamId), versionNumber(versionNumber)
{
}

SdtTable::~SdtTable()
{
}

void SdtTable::AddService(ServiceId serviceId, uchar_t eitScheduleFlag, 
                          uchar_t eitPresentFollowingFlag, uint16_t runningStatus, uint16_t freeCaMode)
{
    SdtService *sdtService = new SdtService(serviceId, eitScheduleFlag, eitPresentFollowingFlag, 
                                            runningStatus, freeCaMode);
    sdtServices.AddSdtService(sdtService);
}

void SdtTable::AddServiceDescriptor(ServiceId serviceId, std::string &data)
{
    Descriptor* descriptor = CreateDescriptor(data);
    if (descriptor == nullptr)
    {
        errstrm << "we do not support descriptor whose tag = " 
            << hex << data[0] << data[1] << endl;
        return;
    }

    sdtServices.AddServiceDescriptor(serviceId, descriptor);
}

size_t SdtTable::GetCodesSize(TableId tableId, const TsIds &tsIds,
                              SectionNumber secIndex) const
{
    if (this->tableId != tableId)
        return 0;

    TsIds::const_iterator iter;
    iter = find(tsIds.cbegin(), tsIds.cend(), transportStreamId);
    if (iter == tsIds.cend())
        return 0;

    //check secIndex is valid.
    assert(secIndex < GetSecNumber(tableId, tsIds));

    size_t serviceOffset = 0;
    for (SectionNumber i = 0; i < secIndex; ++i)
    {
        sdtServices.GetCodesSize(MaxSdtServiceContentSize, serviceOffset);
    }

    size_t size = sdtServices.GetCodesSize(MaxSdtServiceContentSize, serviceOffset);
    return size + sizeof(service_description_section);
}

SiTableKey SdtTable::GetKey() const
{
    return transportStreamId;
}

uint_t SdtTable::GetSecNumber(TableId tableId, const TsIds &tsIds) const
{
    if (this->tableId != tableId)
        return 0;
    
    TsIds::const_iterator iter;
    iter = find(tsIds.cbegin(), tsIds.cend(), transportStreamId);
    if (iter == tsIds.cend())
        return 0;

    uint_t secNumber = 1;
    size_t serviceOffset = 0;
    sdtServices.GetCodesSize(MaxSdtServiceContentSize, serviceOffset);

    size_t tsSize;
    for (tsSize = sdtServices.GetCodesSize(MaxSdtServiceContentSize, serviceOffset);
         tsSize != 0;
         tsSize = sdtServices.GetCodesSize(MaxSdtServiceContentSize, serviceOffset))
    {
        ++secNumber;
    }

    return secNumber;
}

TableId SdtTable::GetTableId() const
{
    return tableId;
}

size_t SdtTable::MakeCodes(TableId tableId, const TsIds &tsIds, 
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

    size_t serviceOffset = 0;
    SectionNumber secNumber = (SectionNumber)GetSecNumber(tableId, tsIds);
    for (SectionNumber i = 0; i < secIndex; ++i)
    {
        sdtServices.GetCodesSize(MaxSdtServiceContentSize, serviceOffset);
    }

    ptr = ptr + Write8(ptr, tableId);
    WriteHelper<uint16_t> siHelper(ptr, ptr + 2);
    ptr = ptr + Write16(ptr, 0); 
    ptr = ptr + Write16(ptr, transportStreamId); //transport_stream_id

    uchar_t currentNextIndicator = 1;
    //version_number
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator); 
    ptr = ptr + Write8(ptr, secIndex);           //section_number
    ptr = ptr + Write8(ptr, secNumber - 1);      //last_section_number
    ptr = ptr + Write16(ptr, originalNetworkId); //original_network_id
    ptr = ptr + Write8(ptr, Reserved8Bit);

    ptr = ptr + sdtServices.MakeCodes(ptr, MaxSdtServiceContentSize, serviceOffset);

    siHelper.Write((SdtSectionSyntaxIndicator << 15) | (Reserved1Bit << 14) | (Reserved2Bit << 12), ptr + 4); 
    ptr = ptr + Write32(ptr, Crc32::CalculateCrc(buffer, ptr - buffer));

    assert(ptr - buffer == size);
    return (ptr - buffer);
}