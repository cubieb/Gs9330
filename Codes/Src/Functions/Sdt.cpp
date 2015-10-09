#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Crc32.h"
#include "Descriptor.h"
#include "XmlDataWrapper.h"
#include "Sdt.h"

using namespace std;

/**********************class SdtService**********************/
SdtService::SdtService(uint16_t serviceId, uchar_t eitScheduleFlag, 
                       uchar_t eitPresentFollowingFlag, uint16_t runningStatus, 
                       uint16_t freeCaMode)
    : serviceId(serviceId), eitScheduleFlag(eitScheduleFlag), 
      eitPresentFollowingFlag(eitPresentFollowingFlag),
      runningStatus(runningStatus), freeCaMode(freeCaMode)
{
    descriptors.reset(new Descriptors);
}

uint16_t SdtService::GetServiceId()
{
    return serviceId;
}

void SdtService::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    descriptors->AddDescriptor(tag, data, dataSize);
}

void SdtService::AddServiceDescriptor0x48(uchar_t serviceType, uchar_t *providerName, uchar_t *serviceName)
{
    descriptors->AddDescriptor0x48(serviceType, providerName, serviceName);
}

size_t SdtService::GetCodesSize() const
{
    return (descriptors->GetCodesSize() + 3);
}

size_t SdtService::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    size_t size = GetCodesSize();
    assert(size <= bufferSize);
    
    ptr = ptr + Write16(ptr, serviceId);
    ptr = ptr + Write8(ptr, (Reserved6Bit << 2) | (eitScheduleFlag << 1) | eitPresentFollowingFlag);
    ptr = ptr + descriptors->MakeCodes(ptr, bufferSize - 3, (runningStatus << 1) | freeCaMode);
    
    assert(ptr - buffer == size);
    return (ptr - buffer);
}

void SdtService::Put(std::ostream& os) const
{
    os << "service_id = " << (uint_t)serviceId
        << ", EIT_schedule_flag = " << (uint_t)eitScheduleFlag
        << ", EIT_present_following_flag = " << (uint_t)eitPresentFollowingFlag  
        << ", running_status = " << (uint_t)runningStatus  
        << ", free_CA_mode = " << (uint_t)freeCaMode  
        << endl;

    os << *descriptors;
}

/**********************class SdtServices**********************/
size_t SdtServices::GetCodesSize() const
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

size_t SdtServices::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;  
    size_t size = GetCodesSize();
    assert(size <= bufferSize);

    for (const auto iter: components)
    {
        ptr = ptr + iter->MakeCodes(ptr, bufferSize - (ptr - buffer));
    }
    assert(ptr - buffer == size);
    return (ptr - buffer);
}

void SdtServices::AddSdtService(uint16_t serviceId, uchar_t eitScheduleFlag, 
    uchar_t eitPresentFollowingFlag, uint16_t runningStatus, 
    uint16_t freeCaMode)
{
    auto service = make_shared<SdtService>(serviceId, eitScheduleFlag,
        eitPresentFollowingFlag, runningStatus, freeCaMode);
    AddComponent(service);
}

void SdtServices::AddServiceDescriptor(uint16_t serviceId, uchar_t tag, uchar_t* data, size_t dataSize)
{
    auto iter = find_if(components.begin(), components.end(), EqualSdtService(serviceId));
    SdtService& service = dynamic_cast<SdtService&>(**iter);
    service.AddDescriptor(tag, data, dataSize);
}

void SdtServices::AddServiceDescriptor0x48(uint16_t serviceId, uchar_t serviceType, 
                                           uchar_t *providerName, uchar_t *serviceName)
{
    auto iter = find_if(components.begin(), components.end(), EqualSdtService(serviceId));
    SdtService& service = dynamic_cast<SdtService&>(**iter);
    service.AddServiceDescriptor0x48(serviceType, providerName, serviceName);
}

void SdtServices::Put(std::ostream& os) const
{
    MyBase::Put(os);
}

/**********************class Sdt**********************/
Sdt::Sdt(const char *key)
    : Section(key), tableId(0), transportStreamId(0), versionNumber(0), 
        sectionNumber(0), lastSectionNumber(0), originalNetworkId(0)
{
    services.reset(new SdtServices);
}

Sdt::Sdt(const char *key, uchar_t *buffer)
    : Section(key)
{
    services.reset(new SdtServices);

    uchar_t *ptr = buffer;
    uint16_t sectionLength;
    ptr = ptr + Read8(ptr, tableId);
    ptr = ptr + Read16(ptr, sectionLength);
    sectionLength = sectionLength & 0xfff;

    ptr = ptr + Read16(ptr, transportStreamId);
    ptr = ptr + Read8(ptr, versionNumber);
    versionNumber = (versionNumber & 0x2f) >> 1;
    
    ptr = ptr + Read8(ptr, sectionNumber);
    ptr = ptr + Read8(ptr, lastSectionNumber);
    ptr = ptr + Read16(ptr, originalNetworkId);
    SetNetworkId(originalNetworkId);
    ptr = ptr + 1; //reserved_future_use

    while (ptr < buffer + sectionLength - 1)
    {
        uint16_t value16, serviceId, desLength;
        uchar_t  value8, eitScheduleFlag, eitPresentFollowingFlag, runningStatus, freeCaMode;
        ptr = ptr + Read16(ptr, serviceId);
        ptr = ptr + Read8(ptr, value8);
        eitScheduleFlag = (value8 >> 1) & 0x1;
        eitPresentFollowingFlag = value8 & 0x1;
        ptr = ptr + Read16(ptr, value16);
        runningStatus = value16 >> 13;
        freeCaMode = (value16 >> 12) & 0x1;

        AddService(serviceId, eitScheduleFlag, eitPresentFollowingFlag, runningStatus, freeCaMode);

        desLength = value16 & 0xfff;
        if (desLength != 0)
        {
            AddServiceDescriptor(serviceId, ptr[0], ptr+2, desLength - 2);
            ptr = ptr + desLength;
        }
    }
}

uint16_t Sdt::GetPid() const
{
    return Pid;
}

void Sdt::SetTableId(uchar_t data)
{
    /* Table 2: Allocation of table_id values */
    assert(data == 0x42 || data == 0x46);
    tableId = data;
}

uchar_t Sdt::GetTableId() const
{
    return tableId;
}

void Sdt::SetNetworkId(uint16_t data)
{
    networkId = data;
}

uint16_t Sdt::GetNetworkId() const
{
    return networkId;
}

void Sdt::SetTsId(uint16_t data)
{
    transportStreamId = data;
}

void Sdt::SetVersionNumber(uchar_t data)
{
    versionNumber = data;
}

void Sdt::SetSectionNumber(uchar_t data)
{
    sectionNumber = data;
}

void Sdt::SetLastSectionNumber(uchar_t data)
{
    lastSectionNumber = data;
}

void Sdt::SetOnId(uint16_t data)
{
    originalNetworkId = data;
}

void Sdt::AddService(uint16_t serviceId, uchar_t eitScheduleFlag, 
                     uchar_t eitPresentFollowingFlag, uint16_t runningStatus, 
                     uint16_t freeCaMode)
{
    services->AddSdtService(serviceId, eitScheduleFlag, eitPresentFollowingFlag, 
                            runningStatus, freeCaMode);
}

void Sdt::AddServiceDescriptor(uint16_t serviceId, uchar_t tag, uchar_t* data, size_t dataSize)
{
    services->AddServiceDescriptor(serviceId, tag, data, dataSize);
}

void Sdt::AddServiceDescriptor0x48(uint16_t serviceId, uchar_t serviceType, uchar_t *providerName, uchar_t *serviceName)
{
    services->AddServiceDescriptor0x48(serviceId, serviceType, providerName, serviceName);
}

size_t Sdt::GetCodesSize() const
{
    return services->GetCodesSize() + sizeof(service_description_section);
}

size_t Sdt::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    uint16_t ui16Value; 
    size_t  size = GetCodesSize();

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

    ptr = ptr + services->MakeCodes(ptr, bufferSize - sizeof(service_description_section));

    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.CalculateCrc(buffer, ptr - buffer));
    assert(ptr - buffer == size);
    return (ptr - buffer);
}

void Sdt::Put(std::ostream& os) const
{
    ios::fmtflags flags = cout.flags( );
    os  << "table_id = " << showbase << hex <<(uint_t)tableId
        << ", section_length = " << dec << (uint_t)GetCodesSize() - 3
        << ", transport_stream_id = " << (uint_t)transportStreamId
        << ", version_number = " << hex << (uint_t)versionNumber 
        << endl ;
    cout.flags(flags);

    os << *services;
}