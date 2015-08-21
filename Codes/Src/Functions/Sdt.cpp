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
}

void SdtService::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    descriptors->AddDescriptor(tag, data, dataSize);
}

size_t SdtService::GetCodesSize() const
{
    return (descriptors->GetCodesSize() + 5);
}

size_t SdtService::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;
    assert(GetCodesSize() <= bufferSize);

    ptr = ptr + Write16(ptr, serviceId);
    ptr = ptr + Write8(ptr, (Reserved6Bit << 2) | (eitScheduleFlag << 1) | eitPresentFollowingFlag);
    ptr = ptr + Write16(ptr, (runningStatus << 13) | (freeCaMode << 12) | descriptors->GetCodesSize());

    ptr = ptr + descriptors->MakeCodes(ptr, bufferSize);

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
size_t SdtServices::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    return 0;
}

void SdtServices::AddSdtService(uint16_t serviceId, uchar_t eitScheduleFlag, 
    uchar_t eitPresentFollowingFlag, uint16_t runningStatus, 
    uint16_t freeCaMode)
{
}

void SdtServices::AddServiceDescriptor(uint16_t tsId, uchar_t tag, uchar_t* data, size_t dataSize)
{
}

void SdtServices::Put(std::ostream& os) const
{
    MyBase::Put(os);
}

/**********************class Sdt**********************/
Sdt::Sdt(): tableId(0), transportStreamId(0), versionNumber(0), 
            sectionNumber(0), lastSectionNumber(0), originalNetworkId(0)
{
    services.reset(new SdtServices);
}

uint16_t Sdt::GetPid() const
{
    return Pid;
}

void Sdt::SetTableId(uchar_t data)
{
    tableId = data;
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

size_t Sdt::GetCodesSize() const
{
    return 0;
}

size_t Sdt::MakeCodes(uchar_t *buffer, size_t bufferSize)
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

    ptr = ptr + services->MakeCodes(ptr, bufferSize);

    Crc32 crc32;
    ptr = ptr + Write32(ptr, crc32.CalculateCrc(buffer, ptr - buffer));
    return (ptr - buffer);
}

void Sdt::Put(std::ostream& os) const
{
    os  << "tableId = " << (uint_t)tableId << endl ;
}