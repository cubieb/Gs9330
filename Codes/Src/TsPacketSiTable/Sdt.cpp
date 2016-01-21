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

SdtTableInterface * SdtTableInterface::CreateInstance(TableId tableId, TsId transportStreamId, Version versionNumber, NetId originalNetworkId)
{
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
    return (descriptors.GetCodesSize() + 3);
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
    ptr = ptr + descriptors.MakeCodes(ptr, bufferSize - 3, (runningStatus << 1) | freeCaMode);
    
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

size_t SdtServices::GetCodesSize() const
{    
    /* there is no reserved_future_use and xxx_xxx__length fields, so we
       have to impliment GetCodesSize() and MakeCodes() myself.
     */
    size_t size = 0;
    for (const auto iter: sdtServices)
    {
        size = size + iter->GetCodesSize();
    }
        
    return size; 
}

size_t SdtServices::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;  
    size_t size = GetCodesSize();
    assert(size <= bufferSize);

    for (const auto iter: sdtServices)
    {
        ptr = ptr + iter->MakeCodes(ptr, bufferSize - (ptr - buffer));
    }
    assert(ptr - buffer == size);
    return (ptr - buffer);
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

/**********************class SdtTable**********************/
/* public function */
SdtTable::SdtTable(TableId tableId, TsId transportStreamId, Version versionNumber, NetId originalNetworkId)
    : tableId(tableId), transportStreamId(transportStreamId), versionNumber(versionNumber), 
      sectionNumber(0), lastSectionNumber(0), originalNetworkId(originalNetworkId) 
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

size_t SdtTable::GetCodesSize(TableId tableId, const std::list<TsId>& tsIds) const
{
    if (this->tableId != tableId)
        return 0;

    std::list<TsId>::const_iterator iter;
    iter = find(tsIds.cbegin(), tsIds.cend(), transportStreamId);
    if (iter == tsIds.cend())
        return 0;

    return sdtServices.GetCodesSize() + sizeof(service_description_section);
}

uint16_t SdtTable::GetKey() const
{
    return transportStreamId;
}

TableId SdtTable::GetTableId() const
{
    return tableId;
}

size_t SdtTable::MakeCodes(TableId tableId, std::list<TsId>& tsIds, 
                           uchar_t *buffer, size_t bufferSize) const
{    
    uchar_t *ptr = buffer;
    uint16_t ui16Value; 
    size_t  size = GetCodesSize(tableId, tsIds);
    if (size == 0)
        return 0;

    assert(size <= bufferSize && size <= (MaxSdtSectionLength - 3));
    ptr = ptr + Write8(ptr, tableId);
    ui16Value = (SdtSectionSyntaxIndicator << 15) | (Reserved1Bit << 14) | (Reserved2Bit << 12) | (size - 3);
    ptr = ptr + Write16(ptr, ui16Value);         //section_length
    ptr = ptr + Write16(ptr, transportStreamId); //transport_stream_id

    uchar_t currentNextIndicator = 1;
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator); //version_number
    ptr = ptr + Write8(ptr, sectionNumber);      //section_number
    ptr = ptr + Write8(ptr, lastSectionNumber);  //last_section_number
    ptr = ptr + Write16(ptr, originalNetworkId); //original_network_id
    ptr = ptr + Write8(ptr, Reserved8Bit);

    ptr = ptr + sdtServices.MakeCodes(ptr, bufferSize - sizeof(service_description_section));

    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.CalculateCrc(buffer, ptr - buffer));
    assert(ptr - buffer == size);
    return (ptr - buffer);
}