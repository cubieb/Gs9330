#include "Include/Foundation/SystemInclude.h"

/* Foundation */
#include "Include/Foundation/Type.h"
#include "Include/Foundation/Deleter.h"
#include "Include/Foundation/PacketHelper.h"
#include "Include/Foundation/Converter.h"
#include "Include/Foundation/Crc32.h"
#include "Include/Foundation/Deleter.h"

/* TsPacketSiTable */
#include "Descriptor.h"
using namespace std;

/**********************class Descriptor**********************/
Descriptor::Descriptor(uchar_t *theData)
{
    data = new uchar_t[theData[1] + 2];
    memcpy(data, theData, theData[1] + 2);
}

Descriptor::~Descriptor()
{
    delete[] data;
}

size_t Descriptor::GetCodesSize() const
{
    return data[1] + 2;
}

uchar_t Descriptor::GetTag() const
{
    return data[0];
}

size_t Descriptor::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    size_t size = GetCodesSize();
    assert(size <= bufferSize);
    Write(buffer, bufferSize, data, size);

    return size;
}

/**********************class DescriptorCreator**********************/
DescriptorCreator::DescriptorCreator() 
{}

DescriptorCreator::~DescriptorCreator() 
{}

/* Create descriptor from string. */
Descriptor* DescriptorCreator::CreateInstance(std::string &strData) 
{ 
    return nullptr; 
}

/* Create descriptor from binary (for example .ts file). */
Descriptor* DescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return nullptr; 
}

/**********************class NetworkNameDescriptor**********************/
NetworkNameDescriptor::NetworkNameDescriptor(uchar_t *data)
        : Descriptor(data)
{}

Descriptor* NetworkNameDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x40);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;        

    std::string networkName;
    data =  data + ConvertUtf8ToString(data, networkName, descriptorLenght);

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)networkName.size());
    ptr = ptr + WriteBuffer(ptr, networkName);

    return new NetworkNameDescriptor(buffer);
}

Descriptor* NetworkNameDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new NetworkNameDescriptor(data); 
}

/**********************class ServiceListDescriptor**********************/
ServiceListDescriptor::ServiceListDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * ServiceListDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x41);

    assert(descriptorLenght % 3 == 0);
    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;
    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);

    for (size_t i = 0; i < descriptorLenght; i = i + 3)
    {
        uint16_t serviceId;
        uchar_t  serviceType;

        data = data + ConvertHexStrToInt(data, serviceId);
        data = data + ConvertHexStrToInt(data, serviceType);

        ptr = ptr + WriteBuffer(ptr, serviceId);
        ptr = ptr + WriteBuffer(ptr, serviceType);
    }

    return new ServiceListDescriptor(buffer);
}

Descriptor * ServiceListDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new ServiceListDescriptor(data); 
}

/**********************class StuffingDescriptor**********************/
StuffingDescriptor::StuffingDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * StuffingDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x42);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;
    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);

    ptr = ptr + ConvertStrToIntStr(data, descriptorLenght * 2, ptr);

    return new StuffingDescriptor(buffer);
}

Descriptor * StuffingDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new StuffingDescriptor(data); 
}

/**********************class SatelliteDeliverySystemDescriptor**********************/
SatelliteDeliverySystemDescriptor::SatelliteDeliverySystemDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * SatelliteDeliverySystemDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x43);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;
    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);

    uint32_t frequency;
    uint16_t orbitalPosition;
    uchar_t  weastEastFlag; //west_east_flag + polarization + roll off/"00" + modulation_system + modulation_type
    uint32_t symbolRate;    //symbol_rate + FEC_inner

    data = data + ConvertHexStrToInt(data, frequency);
    data = data + ConvertHexStrToInt(data, orbitalPosition);
    data = data + ConvertHexStrToInt(data, weastEastFlag);
    data = data + ConvertHexStrToInt(data, symbolRate);

    ptr = ptr + WriteBuffer(ptr, frequency);
    ptr = ptr + WriteBuffer(ptr, orbitalPosition);
    ptr = ptr + WriteBuffer(ptr, weastEastFlag);
    ptr = ptr + WriteBuffer(ptr, symbolRate);

    return new SatelliteDeliverySystemDescriptor(buffer);
}

Descriptor * SatelliteDeliverySystemDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new SatelliteDeliverySystemDescriptor(data); 
}

/**********************class CableDeliverySystemDescriptor**********************/
CableDeliverySystemDescriptor::CableDeliverySystemDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * CableDeliverySystemDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x44);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;
    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);

    uint32_t frequency;
    uint16_t reserved; //reserved_future_use + FEC_outer
    uchar_t  modulation; 
    uint32_t symbolRate;   

    data = data + ConvertHexStrToInt(data, frequency);
    data = data + ConvertHexStrToInt(data, reserved);
    data = data + ConvertHexStrToInt(data, modulation);
    data = data + ConvertHexStrToInt(data, symbolRate);

    ptr = ptr + WriteBuffer(ptr, frequency);
    ptr = ptr + WriteBuffer(ptr, reserved);
    ptr = ptr + WriteBuffer(ptr, modulation);
    ptr = ptr + WriteBuffer(ptr, symbolRate);

    return new CableDeliverySystemDescriptor(buffer);
}

Descriptor * CableDeliverySystemDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new CableDeliverySystemDescriptor(data); 
}

/**********************class BouquetNameDescriptor**********************/
BouquetNameDescriptor::BouquetNameDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * BouquetNameDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x47);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;        

    std::string bouquetName;
    data =  data + ConvertUtf8ToString(data, bouquetName, descriptorLenght);

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)bouquetName.size());
    ptr = ptr + WriteBuffer(ptr, bouquetName);

    return new BouquetNameDescriptor(buffer);
}

Descriptor * BouquetNameDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new BouquetNameDescriptor(data); 
}

/**********************class ServiceDescriptor**********************/
ServiceDescriptor::ServiceDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * ServiceDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x48);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)0);  //fill a '0' as pad. write value at end of this function.

    uchar_t serviceType, serviceProviderNameLength, serviceNameLength;
    std::string serviceProviderName, serviceName;        

    data = data + ConvertHexStrToInt(data, serviceType);
    data = data + ConvertHexStrToInt(data, serviceProviderNameLength);
    data = data + ConvertUtf8ToString(data, serviceProviderName, serviceProviderNameLength);
    data = data + ConvertHexStrToInt(data, serviceNameLength);
    data = data + ConvertUtf8ToString(data, serviceName, serviceNameLength);        

    ptr = ptr + WriteBuffer(ptr, serviceType);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)serviceProviderName.size());
    ptr = ptr + WriteBuffer(ptr, serviceProviderName);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)serviceName.size());
    ptr = ptr + WriteBuffer(ptr, serviceName);

    //rewrite the descripor length.
    WriteBuffer(buffer + 1, uchar_t(ptr - buffer - 2));

    return new ServiceDescriptor(buffer);  
}

Descriptor * ServiceDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new ServiceDescriptor(data); 
}

/**********************class LinkageDescriptor**********************/
LinkageDescriptor::LinkageDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * LinkageDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x4A);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);

    uint16_t tsId;
    uint16_t onId;
    uint16_t serviceId;
    uchar_t  linkageType;

    data = data + ConvertHexStrToInt(data, tsId);
    data = data + ConvertHexStrToInt(data, onId);
    data = data + ConvertHexStrToInt(data, serviceId);
    data = data + ConvertHexStrToInt(data, linkageType);  

    ptr = ptr + WriteBuffer(ptr, tsId);
    ptr = ptr + WriteBuffer(ptr, onId);
    ptr = ptr + WriteBuffer(ptr, serviceId);
    ptr = ptr + WriteBuffer(ptr, linkageType);

    if (linkageType == 0x08)
    {
        uchar_t value8; //hand-over_type + reserved_future_use + origin_type
        data = data + ConvertHexStrToInt(data, value8);
        ptr = ptr + WriteBuffer(ptr, value8);
        uchar_t handOverType = (value8 >> 4) & 0xf;
        if (handOverType == 0x1 || handOverType == 0x2 || handOverType == 0x3)
        {
            uint16_t netId;
            data = data + ConvertHexStrToInt(data, netId);
            ptr = ptr + WriteBuffer(ptr, netId);
        }

        uchar_t originType = value8 & 0x1;;
        if (originType == 0x0)
        {
            uint16_t initialServiceId;
            data = data + ConvertHexStrToInt(data, initialServiceId);
            ptr = ptr + WriteBuffer(ptr, initialServiceId);
        }
    }
    else if (linkageType == 0x0D)
    {
        uint16_t targetEventId;
        data = data + ConvertHexStrToInt(data, targetEventId);
        ptr = ptr + WriteBuffer(ptr, targetEventId);

        uchar_t value8; //target_listed + event_simulcast + reserved
        data = data + ConvertHexStrToInt(data, value8);
        ptr = ptr + WriteBuffer(ptr, value8);
    }
    else if (linkageType == 0x0E)
    { 
        uchar_t loopLength;
        data = data + ConvertHexStrToInt(data, loopLength);
        ptr = ptr + WriteBuffer(ptr, loopLength);
        uchar_t *endPtr = data + loopLength;
        while(data < endPtr)
        {
            //  target_listed + event_simulcast + link_type + target_id_type 
            //+ original_network_id_flag + service_id_flag
            uchar_t value8; 
            data = data + ConvertHexStrToInt(data, value8);
            ptr = ptr + WriteBuffer(ptr, value8);

            uchar_t targetIdType = (value8 >> 4) & 0x2;
            uchar_t originalNetworkIdFlag = (value8 >> 1) & 0x1;
            uchar_t serviceIdFlag = value8 & 0x1;
            if (targetIdType == 0x3)
            {
                uint16_t userDefinedId;
                data = data + ConvertHexStrToInt(data, userDefinedId);
                ptr = ptr + WriteBuffer(ptr, userDefinedId);
            }
            else
            {
                if (targetIdType == 0x1)
                {
                    uint16_t targetTransportStreamId;
                    data = data + ConvertHexStrToInt(data, targetTransportStreamId);
                    ptr = ptr + WriteBuffer(ptr, targetTransportStreamId);
                }
                if (originalNetworkIdFlag != 0)
                {
                    uint16_t targetOriginalNetworkId;
                    data = data + ConvertHexStrToInt(data, targetOriginalNetworkId);
                    ptr = ptr + WriteBuffer(ptr, targetOriginalNetworkId);
                }
                if (serviceIdFlag != 0)
                {
                    uint16_t targetServiceId;
                    data = data + ConvertHexStrToInt(data, targetServiceId);
                    ptr = ptr + WriteBuffer(ptr, targetServiceId);
                }
            }
        }
    }

    uchar_t privateDataTypeLength = descriptorLenght - (ptr - (buffer + 2));
    ptr = ptr + ConvertStrToIntStr(data, privateDataTypeLength * 2, ptr);

    assert(ptr - buffer <= UCHAR_MAX);
    return new LinkageDescriptor(buffer);
}

Descriptor * LinkageDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new LinkageDescriptor(data); 
}

/**********************class ShortEventDescriptor**********************/
ShortEventDescriptor::ShortEventDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor* ShortEventDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x4D);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)0);  //fill a '0' as pad. write value at end of this function.

    uchar_t languageCode[3];
    uchar_t eventNameLength;
    std::string eventName;
    uchar_t textLength;
    std::string text;
    data = data + Write(languageCode, 3, data, 3);
    data = data + ConvertHexStrToInt(data, eventNameLength);
    data = data + ConvertUtf8ToString(data, eventName, eventNameLength);
    data = data + ConvertHexStrToInt(data, textLength);
    data = data + ConvertUtf8ToString(data, text, textLength);     

    ptr = ptr + Write(ptr, 3, languageCode, 3);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)eventName.size());
    ptr = ptr + WriteBuffer(ptr, eventName);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)text.size());
    ptr = ptr + WriteBuffer(ptr, text);

    //rewrite the descripor length.
    WriteBuffer(buffer + 1, uchar_t(ptr - buffer - 2));

    return new ShortEventDescriptor(buffer);
}

Descriptor * ShortEventDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new ShortEventDescriptor(data); 
}

/**********************class ExtendedEventDescriptor**********************/
ExtendedEventDescriptor::ExtendedEventDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * ExtendedEventDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    assert(tag == 0x4E);
    data = data + ConvertHexStrToInt(data, descriptorLenght);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)0);  //fill a '0' as pad. write value at end of this function.

    uchar_t descriptorNumber;  //descriptor_number + last_descriptor_number
    uchar_t languageCode[3];
    uchar_t lengthOfItem;      
    uchar_t textLength;        
    std::string text;

    data = data + ConvertHexStrToInt(data, descriptorNumber);
    ptr = ptr + WriteBuffer(ptr, descriptorNumber);

    data = data + Write(languageCode, 3, data, 3);
    ptr = ptr + Write(ptr, 3, languageCode, 3);

    data = data + ConvertHexStrToInt(data, lengthOfItem);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)0); //fill a '0' as pad. write value at end of this function.

    int leftByteNumber = lengthOfItem;
    while (leftByteNumber > 0)
    {
        uchar_t itemDescriptionLength, itemLength;
        std::string itemDescriptionChar, itemChar;

        data = data + ConvertHexStrToInt(data, itemDescriptionLength);
        data = data + ConvertUtf8ToString(data, itemDescriptionChar, itemDescriptionLength);
        ptr = ptr + WriteBuffer(ptr, (uchar_t)itemDescriptionChar.size());
        ptr = ptr + WriteBuffer(ptr, itemDescriptionChar);

        data = data + ConvertHexStrToInt(data, itemLength);
        data = data + ConvertUtf8ToString(data, itemChar, itemLength);
        ptr = ptr + WriteBuffer(ptr, (uchar_t)itemChar.size());
        ptr = ptr + WriteBuffer(ptr, itemChar);

        leftByteNumber = leftByteNumber - 2 - itemDescriptionLength - itemLength;
    }
    assert(leftByteNumber == 0);
    //rewrite the iterm length.
    WriteBuffer(buffer + 6, (uchar_t)(ptr - buffer - 7));

    data = data + ConvertHexStrToInt(data, textLength);
    data = data + ConvertUtf8ToString(data, text, textLength);

    ptr = ptr + WriteBuffer(ptr, (uchar_t)text.size());
    ptr = ptr + WriteBuffer(ptr, text);

    WriteBuffer(buffer + 1, uchar_t(ptr - buffer - 2));

    return new ExtendedEventDescriptor(buffer);
}

Descriptor * ExtendedEventDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new ExtendedEventDescriptor(data); 
}

/**********************class TimeShiftedEventDescriptor**********************/
TimeShiftedEventDescriptor::TimeShiftedEventDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * TimeShiftedEventDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x4F);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght); 
              
    uint16_t referenceServiceId, referenceEventId;
    data = data + ConvertHexStrToInt(data, referenceServiceId);
    data = data + ConvertHexStrToInt(data, referenceEventId);

    ptr = ptr + WriteBuffer(ptr, referenceServiceId);
    ptr = ptr + WriteBuffer(ptr, referenceEventId); 

    return new TimeShiftedEventDescriptor(buffer);
}

Descriptor * TimeShiftedEventDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new TimeShiftedEventDescriptor(data); 
}

/**********************class ComponentDescriptor**********************/
ComponentDescriptor::ComponentDescriptor(uchar_t *data)
    : Descriptor(data)
{}

 Descriptor * ComponentDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x50);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght); 
              
    uchar_t streamContent, componentType, componentTag;
    data = data + ConvertHexStrToInt(data, streamContent);
    data = data + ConvertHexStrToInt(data, componentType);
    data = data + ConvertHexStrToInt(data, componentTag);

    ptr = ptr + WriteBuffer(ptr, streamContent);
    ptr = ptr + WriteBuffer(ptr, componentType);
    ptr = ptr + WriteBuffer(ptr, componentTag); 

    uchar_t languageCode[3];
    std::string text;
    data = data + Write(languageCode, 3, data, 3);
    data = data + ConvertUtf8ToString(data, text, descriptorLenght - 6);

    ptr = ptr + Write(ptr, 3, languageCode, 3);
    ptr = ptr + WriteBuffer(ptr, text);

    return new ComponentDescriptor(buffer);
}

Descriptor * ComponentDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new ComponentDescriptor(data); 
}

/**********************class CaIdentifierDescriptor**********************/
CaIdentifierDescriptor::CaIdentifierDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * CaIdentifierDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x53);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght); 

    while (ptr < buffer + 2 + descriptorLenght)
    {
        uint16_t caSystemId;
        data = data + ConvertHexStrToInt(data, caSystemId);
        ptr = ptr + WriteBuffer(ptr, caSystemId);
    }        

    return new CaIdentifierDescriptor(buffer);
}

Descriptor * CaIdentifierDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new CaIdentifierDescriptor(data); 
}

/**********************class ContentDescriptor**********************/
ContentDescriptor::ContentDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * ContentDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x54);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght); 

    while (ptr < buffer + 2 + descriptorLenght)
    {
        uchar_t content_nibble_level, user_byte;
        data = data + ConvertHexStrToInt(data, content_nibble_level);
        data = data + ConvertHexStrToInt(data, user_byte);

        ptr = ptr + WriteBuffer(ptr, content_nibble_level);
        ptr = ptr + WriteBuffer(ptr, user_byte);
    }        

    return new ContentDescriptor(buffer);
}

Descriptor * ContentDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new ContentDescriptor(data); 
}

/**********************class ParentalRatingDescriptor**********************/
ParentalRatingDescriptor::ParentalRatingDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * ParentalRatingDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x55);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght); 

    while (ptr < buffer + 2 + descriptorLenght)
    {
        uchar_t countryCode[3];
        uchar_t rating;
        data = data + Write(countryCode, 3, data, 3);
        data = data + ConvertHexStrToInt(data, rating);

        ptr = ptr + Write(ptr, 3, countryCode, 3);
        ptr = ptr + WriteBuffer(ptr, rating);
    }        

    return new ParentalRatingDescriptor(buffer);
}

Descriptor * ParentalRatingDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new ParentalRatingDescriptor(data); 
}

/**********************class TerrestrialDeliverySystemDescriptor**********************/
TerrestrialDeliverySystemDescriptor::TerrestrialDeliverySystemDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * TerrestrialDeliverySystemDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x5A);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);

    uchar_t  value8;
    uint16_t value16;
    uint32_t value32;
    data = data + ConvertHexStrToInt(data, value32); //centre_frequency
    ptr = ptr + WriteBuffer(ptr, value32);

    //bandwidth 3 + priority 1 + Time_Slicing_indicator 1 + MPE-FEC_indicator 1 + reserved_future_use 2
    data = data + ConvertHexStrToInt(data, value8); 
    ptr = ptr + WriteBuffer(ptr, value8);

    //constellation 2 + hierarchy_information 3 + code_rate-HP_stream 3 + code_rate-LP_stream 3 
    // + guard_interval 2 + transmission_mode 2 + other_frequency_flag 1
    data = data + ConvertHexStrToInt(data, value16); 
    ptr = ptr + WriteBuffer(ptr, value16);

    //reserved_future_use 32
    data = data + ConvertHexStrToInt(data, value32); 
    ptr = ptr + WriteBuffer(ptr, value32);
        
    return new TerrestrialDeliverySystemDescriptor(buffer);
}

Descriptor * TerrestrialDeliverySystemDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new TerrestrialDeliverySystemDescriptor(data); 
}

/**********************class MultilingualNetworkNameDescriptor**********************/
MultilingualNetworkNameDescriptor::MultilingualNetworkNameDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * MultilingualNetworkNameDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x5B);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)0);  //fill a '0' as pad. write value at end of this function.

    int leftByteNumber = descriptorLenght;
    while (leftByteNumber > 0)
    {
        uchar_t languageCode[3];
        data = data + Write(languageCode, 3, data, 3);
        uchar_t networkNameLength;
        std::string text;
        data = data + ConvertHexStrToInt(data, networkNameLength);
        data = data + ConvertUtf8ToString(data, text, networkNameLength);

        ptr = ptr + Write(ptr, 3, languageCode, 3);
        ptr = ptr + WriteBuffer(ptr, (uchar_t)text.size());
        ptr = ptr + WriteBuffer(ptr, text);

        leftByteNumber = leftByteNumber - 4 - networkNameLength;
    }

    //rewrite the descripor length.
    WriteBuffer(buffer + 1, uchar_t(ptr - buffer - 2));

    return new MultilingualNetworkNameDescriptor(buffer);
}

Descriptor * MultilingualNetworkNameDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new MultilingualNetworkNameDescriptor(data); 
}

/**********************class MultilingualComponentDescriptor**********************/
MultilingualComponentDescriptor::MultilingualComponentDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * MultilingualComponentDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x5E);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)0);  //fill a '0' as pad. write value at end of this function.  

    uchar_t componentTag;
    data = data + ConvertHexStrToInt(data, componentTag);
    ptr = ptr + WriteBuffer(ptr, componentTag);

    int leftByteNumber = descriptorLenght - 1;
    while (leftByteNumber > 0)
    {
        uchar_t languageCode[3];
        data = data + Write(languageCode, 3, data, 3);
        uchar_t textDescriptionLength;
        std::string text;
        data = data + ConvertHexStrToInt(data, textDescriptionLength);
        data = data + ConvertUtf8ToString(data, text, textDescriptionLength);

        ptr = ptr + Write(ptr, 3, languageCode, 3);
        ptr = ptr + WriteBuffer(ptr, (uchar_t)text.size());
        ptr = ptr + WriteBuffer(ptr, text);

        leftByteNumber = leftByteNumber - 4 - textDescriptionLength;
    }

    //rewrite the descripor length.
    WriteBuffer(buffer + 1, uchar_t(ptr - buffer - 2));

    return new MultilingualComponentDescriptor(buffer);
}

Descriptor * MultilingualComponentDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new MultilingualComponentDescriptor(data); 
}

/**********************class PrivateDataSpecifierDescriptor**********************/
PrivateDataSpecifierDescriptor::PrivateDataSpecifierDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * PrivateDataSpecifierDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x5F);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);  

    uint32_t value32;
    data = data + ConvertHexStrToInt(data, value32);
    ptr = ptr + WriteBuffer(ptr, value32);

    return new PrivateDataSpecifierDescriptor(buffer);
}

Descriptor * PrivateDataSpecifierDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new PrivateDataSpecifierDescriptor(data); 
}

/**********************class FrequencyListDescriptor**********************/
FrequencyListDescriptor::FrequencyListDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * FrequencyListDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x62);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);  

    uchar_t reservedFutureUse; //reserved_future_use + coding_type
    data = data + ConvertHexStrToInt(data, reservedFutureUse);
    ptr = ptr + WriteBuffer(ptr, reservedFutureUse);

    while (ptr < buffer + 2 + descriptorLenght)
    {
        uint32_t centreFrequency;
        data = data + ConvertHexStrToInt(data, centreFrequency);
        ptr = ptr + WriteBuffer(ptr, centreFrequency);
    }

    return new FrequencyListDescriptor(buffer);
}

Descriptor* FrequencyListDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new FrequencyListDescriptor(data); 
}

/**********************class DataBroadcastDescriptor**********************/
DataBroadcastDescriptor::DataBroadcastDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * DataBroadcastDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x64);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);  

    uint16_t dataBroadcastId;
    uchar_t componentTag, selectorLength;        
    data = data + ConvertHexStrToInt(data, dataBroadcastId);
    data = data + ConvertHexStrToInt(data, componentTag);
    data = data + ConvertHexStrToInt(data, selectorLength);

    ptr = ptr + WriteBuffer(ptr, dataBroadcastId);
    ptr = ptr + WriteBuffer(ptr, componentTag);
    ptr = ptr + WriteBuffer(ptr, selectorLength);  

    /***/
    uchar_t languageCode[3];
    data = data + Write(languageCode, 3, data, 3);
    uchar_t textLength;
    std::string text;
    data = data + ConvertHexStrToInt(data, textLength);
    data = data + ConvertUtf8ToString(data, text, textLength);

    ptr = ptr + Write(ptr, 3, languageCode, 3);
    ptr = ptr + WriteBuffer(ptr, (uchar_t)text.size());
    ptr = ptr + WriteBuffer(ptr, text);

    return new DataBroadcastDescriptor(buffer);
}

Descriptor * DataBroadcastDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new DataBroadcastDescriptor(data); 
}

/**********************class ExtensionDescriptor**********************/
ExtensionDescriptor::ExtensionDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * ExtensionDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght, descriptorTagExtension;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    data = data + ConvertHexStrToInt(data, descriptorTagExtension);
    assert(tag == 0x7F);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght); 
    ptr = ptr + WriteBuffer(ptr, descriptorTagExtension);  

    if (descriptorTagExtension  == 0x04)
    {
        //T2_delivery_system_descriptor
        uchar_t  value8;
        uint16_t value16;
        uint32_t value32;

        //plp_id
        data = data + ConvertHexStrToInt(data, value8);
        ptr = ptr + WriteBuffer(ptr, value8);

        //T2_system_id
        data = data + ConvertHexStrToInt(data, value16);
        ptr = ptr + WriteBuffer(ptr, value16);

        if (descriptorLenght > 4)
        {
            //SISO/MISO + bandwidth + reserved_future_use + guard_interval
            // + transmission_mode + other_frequency_flag + tfs_flag
            data = data + ConvertHexStrToInt(data, value16);
            ptr = ptr + WriteBuffer(ptr, value16);

            uint16_t tfsFlag = value16 & 0x1;

            while (ptr < buffer + 2 + descriptorLenght)
            {
                //cell_id
                data = data + ConvertHexStrToInt(data, value16);
                ptr = ptr + WriteBuffer(ptr, value16);

                if (tfsFlag == 1)
                {
                    //frequency_loop_length
                    data = data + ConvertHexStrToInt(data, value8);
                    ptr = ptr + WriteBuffer(ptr, value8);
                    for (uint_t i = 0; i < value8; i = i + 4)
                    {
                        //centre_frequency
                        data = data + ConvertHexStrToInt(data, value32);
                        ptr = ptr + WriteBuffer(ptr, value32);
                    }
                }
                else
                {
                    //centre_frequency
                    data = data + ConvertHexStrToInt(data, value32);
                    ptr = ptr + WriteBuffer(ptr, value32);
                }

                //subcell_info_loop_length
                data = data + ConvertHexStrToInt(data, value8);
                ptr = ptr + WriteBuffer(ptr, value8);
                for (uint_t i = 0; i < value8; i = i + 4)
                {
                    //cell_id_extension
                    data = data + ConvertHexStrToInt(data, value8);
                    ptr = ptr + WriteBuffer(ptr, value8);

                    //transposer_frequency
                    data = data + ConvertHexStrToInt(data, value32);
                    ptr = ptr + WriteBuffer(ptr, value32);
                } //for (uint_t i = 0; i < value8; i = i + 4)
            } //while (ptr < buffer + 2 + descriptorLenght)
        } //if (descriptorLenght > 4)
    } //if (descriptorTagExtension  == 0x04)

    return new ExtensionDescriptor(buffer);
}

Descriptor * ExtensionDescriptorCreator::CreateInstance(uchar_t *data) 
{ 
    return new ExtensionDescriptor(data); 
}

/**********************class UserdefinedDscriptor83**********************/
UserdefinedDscriptor83::UserdefinedDscriptor83(uchar_t *data)
    : Descriptor(data)
{}

Descriptor * UserdefinedDscriptor83Creator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    assert(tag == 0x83);

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);  

    uint16_t lcn;  // lcn flag + lcn
    uchar_t vc;    // vc falg + vc

    data = data + ConvertHexStrToInt(data, lcn);
    data = data + ConvertHexStrToInt(data, vc);
    ptr = ptr + WriteBuffer(ptr, lcn);
    ptr = ptr + WriteBuffer(ptr, vc);

    return new UserdefinedDscriptor83(buffer);
}

Descriptor * UserdefinedDscriptor83Creator::CreateInstance(uchar_t *data) 
{ 
    return new UserdefinedDscriptor83(data); 
}

/**********************class UndefinedDescriptor**********************/
UndefinedDescriptor::UndefinedDescriptor(uchar_t *data)
    : Descriptor(data)
{}

Descriptor* UndefinedDescriptorCreator::CreateInstance(std::string &strData)
{
    uchar_t *data = (uchar_t *)strData.c_str();
    uchar_t tag, descriptorLenght;
    data = data + ConvertHexStrToInt(data, tag);
    data = data + ConvertHexStrToInt(data, descriptorLenght);
    if (tag < 0x80)
    {
        /* tag of customized descriptor must >= than 0x80 */
        return nullptr;
    }

    uchar_t buffer[UCHAR_MAX];
    uchar_t *ptr = buffer;

    ptr = ptr + WriteBuffer(ptr, tag);
    ptr = ptr + WriteBuffer(ptr, descriptorLenght);  
    ptr = ptr + ConvertStrToIntStr(data, descriptorLenght * 2, ptr);

    return new UndefinedDescriptor(buffer);
}

Descriptor* UndefinedDescriptorCreator::CreateInstance(uchar_t *data)
{
    return new UndefinedDescriptor(data); 
}

/**********************class Descriptors**********************/
Descriptors::Descriptors()
{
}

Descriptors::~Descriptors()
{
    for_each(descriptors.begin(), descriptors.end(), ScalarDeleter<Descriptor>());
}

void Descriptors::AddDescriptor(Descriptor *discriptor)
{
    descriptors.push_back(discriptor);
}

size_t Descriptors::GetCodesSize() const
{
    /* 2 bytes for reserved_future_use and transport_descriptors_length */
    size_t size = 2;

    for (const auto iter: descriptors)
    {
        size = size + iter->GetCodesSize();
    }
        
    return size;
}

size_t Descriptors::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    uchar_t *ptr = buffer;    
    assert(GetCodesSize() <= bufferSize);

    ptr = ptr + Write16(ptr, 0);
    size_t size = 0;
    for (const auto iter: descriptors)
    {
        ptr = ptr + iter->MakeCodes(ptr, bufferSize - (ptr - buffer));
        size = size + iter->GetCodesSize();
    }
    uint16_t ui16Value = (Reserved4Bit << 12) | size;
    Write16(buffer, ui16Value);

    assert(ptr - buffer == GetCodesSize());
    return (ptr - buffer);
}

/**********************class DescriptorFactory**********************/
Descriptor* DescriptorFactory::Create(std::string &data)
{
    uchar_t tag;
    ConvertHexStrToInt((uchar_t*)data.c_str(), tag);
    auto iter = creators.find(tag);
    if (iter == creators.end())
    {
        UndefinedDescriptorCreator creator;
        return creator.CreateInstance(data);
    }

    DescriptorCreator *creator = iter->second;
    return creator->CreateInstance(data);
}

Descriptor* DescriptorFactory::Create(uchar_t *data)
{
    auto iter = creators.find(data[0]);
    if (iter == creators.end())
    {
        UndefinedDescriptorCreator creator;
        return creator.CreateInstance(data);
    }

    DescriptorCreator *creator = iter->second;
    return creator->CreateInstance(data);
}

void DescriptorFactory::Register(uchar_t tag, DescriptorCreator *creator)
{
    creators.insert(make_pair(tag, creator));
}

/**********************class AutoRegisterSuite**********************/
AutoRegisterSuite::AutoRegisterSuite(uchar_t tag, DescriptorCreator *creator)
{
    DescriptorFactory& factory = DescriptorFactory::GetInstance();
    factory.Register(tag, creator);
}

DescriptorCreatorRegistration(NetworkNameDescriptor::Tag, NetworkNameDescriptorCreator);
DescriptorCreatorRegistration(ServiceListDescriptor::Tag, ServiceListDescriptorCreator);
DescriptorCreatorRegistration(StuffingDescriptor::Tag, StuffingDescriptorCreator);
DescriptorCreatorRegistration(SatelliteDeliverySystemDescriptor::Tag, SatelliteDeliverySystemDescriptorCreator);
DescriptorCreatorRegistration(CableDeliverySystemDescriptor::Tag, CableDeliverySystemDescriptorCreator);
DescriptorCreatorRegistration(BouquetNameDescriptor::Tag, BouquetNameDescriptorCreator);
DescriptorCreatorRegistration(ServiceDescriptor::Tag, ServiceDescriptorCreator);
DescriptorCreatorRegistration(LinkageDescriptor::Tag, LinkageDescriptorCreator);
DescriptorCreatorRegistration(ShortEventDescriptor::Tag, ShortEventDescriptorCreator);
DescriptorCreatorRegistration(ExtendedEventDescriptor::Tag, ExtendedEventDescriptorCreator);
DescriptorCreatorRegistration(TimeShiftedEventDescriptor::Tag, TimeShiftedEventDescriptorCreator);
DescriptorCreatorRegistration(ComponentDescriptor::Tag, ComponentDescriptorCreator);
DescriptorCreatorRegistration(CaIdentifierDescriptor::Tag, CaIdentifierDescriptorCreator);
DescriptorCreatorRegistration(ContentDescriptor::Tag, ContentDescriptorCreator);
DescriptorCreatorRegistration(ParentalRatingDescriptor::Tag, ParentalRatingDescriptorCreator);
DescriptorCreatorRegistration(TerrestrialDeliverySystemDescriptor::Tag, TerrestrialDeliverySystemDescriptorCreator);
DescriptorCreatorRegistration(MultilingualNetworkNameDescriptor::Tag, MultilingualNetworkNameDescriptorCreator);
DescriptorCreatorRegistration(MultilingualComponentDescriptor::Tag, MultilingualComponentDescriptorCreator);
DescriptorCreatorRegistration(PrivateDataSpecifierDescriptor::Tag, PrivateDataSpecifierDescriptorCreator);
DescriptorCreatorRegistration(FrequencyListDescriptor::Tag, FrequencyListDescriptorCreator);
DescriptorCreatorRegistration(DataBroadcastDescriptor::Tag, DataBroadcastDescriptorCreator);
DescriptorCreatorRegistration(ExtensionDescriptor::Tag, ExtensionDescriptorCreator);
DescriptorCreatorRegistration(UserdefinedDscriptor83::Tag, UserdefinedDscriptor83Creator);

/* Create descriptor from string */
Descriptor* CreateDescriptor(std::string &data)
{
    DescriptorFactory& instance = DescriptorFactory::GetInstance();
    return instance.Create(data);
}

/* Create descriptor from binary (for example .ts file). */
Descriptor* CreateDescriptor(uchar_t *data)
{
    DescriptorFactory& instance = DescriptorFactory::GetInstance();
    return instance.Create(data);
}