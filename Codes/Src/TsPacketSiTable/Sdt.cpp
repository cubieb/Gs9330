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

SiTableInterface * SiTableInterface::CreateSdtInstance(TableId tableId, TsId transportStreamId, 
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
    AllocProxy();
}

SdtServices::~SdtServices()
{
    for_each(sdtServices.begin(), sdtServices.end(), ScalarDeleter());
    sdtServices.clear();
    FreeProxy();
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
    : tableId(tableId), transportStreamId(transportStreamId), versionNumber(versionNumber),
      originalNetworkId(originalNetworkId)
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
    ClearCatch();
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
    ClearCatch();
}

SiTableKey SdtTable::GetKey() const
{
    return transportStreamId;
}

TableId SdtTable::GetTableId() const
{
    return tableId;
}

/* protected function */
bool SdtTable::CheckTableId(TableId tableId) const
{
    return (tableId == this->tableId);
}

bool SdtTable::CheckTsId(TsId tsid) const
{
    return (tsid == this->transportStreamId);
}

size_t SdtTable::GetFixedSize() const
{
    return SdtFixedFieldSize;
}

size_t SdtTable::GetVarSize() const
{
    return MaxSdtServiceContentSize;
}

const SdtTable::Var1& SdtTable::GetVar1() const
{
    return varHelper;
}

SdtTable::Var2 SdtTable::GetVar2(TableId tableId) const
{
    return Var2(sdtServices);
}

size_t SdtTable::MakeCodes1(TableId tableId, uchar_t *buffer, size_t bufferSize, size_t var1Size,
                            SectionNumber secNumber, SectionNumber lastSecNumber) const
{
    uchar_t *ptr = buffer;
    
    //section_syntax_indicator:1 + reserved_future_use1:1 + reserved1:2 + section_length:12
    TableSize pseudoSize = 0;
    ptr = ptr + WriteBuffer(ptr, tableId);
    ptr = ptr + WriteBuffer(ptr, pseudoSize); 
    ptr = ptr + WriteBuffer(ptr, transportStreamId); //transport_stream_id

    uchar_t currentNextIndicator = 1;
    //version_number
    ptr = ptr + Write8(ptr, (Reserved2Bit << 6) | (versionNumber << 1) | currentNextIndicator); 
    /* The section_number shall be incremented by 1 with each additional section with the same
       table_id, transport_stream_id, and original_network_id.
     */
    ptr = ptr + Write8(ptr, secNumber);           //section_number
    ptr = ptr + Write8(ptr, lastSecNumber);      //last_section_number
    ptr = ptr + Write16(ptr, originalNetworkId); //original_network_id
    ptr = ptr + Write8(ptr, Reserved8Bit);

    assert(ptr <= buffer + bufferSize);
    return (ptr - buffer);
}

size_t SdtTable::MakeCodes2(Var2 &var2, uchar_t *buffer, size_t bufferSize,
                            size_t var2MaxSize, size_t var2Offset) const
{
    uchar_t *ptr = buffer;

    ptr = ptr + var2.MakeCodes(ptr, var2MaxSize, var2Offset);

    assert(ptr <= buffer + bufferSize);
    return (ptr - buffer);
}