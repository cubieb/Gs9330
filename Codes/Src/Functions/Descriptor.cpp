#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Descriptor.h"

using namespace std;
/**********************class UcharDescriptor**********************/
size_t UcharDescriptor::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    assert(GetCodesSize() <= bufferSize);
    uchar_t *ptr = buffer;
    ptr = ptr + Write8(ptr, GetTag());
    ptr = ptr + Write8(ptr, dataSize);
    ptr = ptr + MemCopy(ptr, bufferSize - 2, data.get(), dataSize);
    return GetCodesSize();
}

size_t UcharDescriptor::GetCodesSize() const
{
    return dataSize + 2;
}

void UcharDescriptor::Put(std::ostream& os) const
{
    ios::fmtflags flags = cout.flags( );
    os << "Descriptor: tag = " << showbase << hex << (uint_t)GetTag() 
        << ", data = " << (char*)data.get()
        << endl;
    cout.flags(flags);
}

/**********************class Descriptors**********************/
void Descriptors::AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize)
{
    Descriptor* ptr = CreateDescriptor(tag, data, dataSize);
    shared_ptr<Descriptor> discripter(ptr);
    if (discripter != nullptr)
    {
        AddComponent(discripter);
    }
    else
        errstrm << "cant create descriptor, tag = " << (uint_t)tag << endl;
}

void Descriptors::AddDescriptor0x41(const std::list<std::pair<uint16_t, uchar_t>>& serviceList)
{
    Descriptor* ptr = new ServiceListDescriptor(serviceList);
    shared_ptr<Descriptor> discripter(ptr);
    if (discripter != nullptr)
    {
        AddComponent(discripter);
    }
    else
        errstrm << "cant create descriptor, tag = " << (uint_t)0x41 << endl;
}

void Descriptors::AddDescriptor0x44(uint32_t frequency, uint16_t fecOuter, 
                                    uchar_t modulation, uint32_t symbolRate, 
                                    uint32_t fecInner)
{
    Descriptor* ptr = new CableDeliverySystemDescriptor(frequency, fecOuter, 
        modulation, symbolRate, fecInner);

    shared_ptr<Descriptor> discripter(ptr);
    if (discripter != nullptr)
    {
        AddComponent(discripter);
    }
    else
        errstrm << "cant create descriptor, tag = " << (uint_t)0x41 << endl;
}

size_t Descriptors::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    return MyBase::MakeCodes(buffer, bufferSize);
}

void Descriptors::Put(std::ostream& os) const
{
    ios::fmtflags flags = cout.flags( );
    os << "reserved_future_use = " << showbase << hex << Reserved4Bit
        << ", network_descriptors_length = " << dec << GetCodesSize() - 2 << endl;
    cout.flags(flags);
    MyBase::Put(os);
}

/**********************class DescriptorFactory**********************/
DescriptorCreatorRgistration(NetworkNameDescriptor::Tag, NetworkNameDescriptor::CreateInstance);
DescriptorCreatorRgistration(ServiceListDescriptor::Tag, 
                             (ServiceListDescriptor::Constructor1)ServiceListDescriptor::CreateInstance);
DescriptorCreatorRgistration(StuffingDescriptor::Tag, StuffingDescriptor::CreateInstance);
DescriptorCreatorRgistration(SatelliteDeliverySystemDescriptor::Tag, SatelliteDeliverySystemDescriptor::CreateInstance);
DescriptorCreatorRgistration(CableDeliverySystemDescriptor::Tag, 
                             (CableDeliverySystemDescriptor::Constructor1)CableDeliverySystemDescriptor::CreateInstance);
DescriptorCreatorRgistration(UserdefinedDscriptor83::Tag, UserdefinedDscriptor83::CreateInstance);

void DescriptorFactory::Register(uchar_t type, DescriptorCreator creator)
{
    creatorMap.insert(make_pair(type, creator));
}

Descriptor* DescriptorFactory::Create(uchar_t type, uchar_t *data, size_t dataSize)
{
    auto iter = creatorMap.find(type);
    if (iter == creatorMap.end())
        return nullptr;

    return iter->second(data, dataSize);
}

Descriptor* CreateDescriptor(uchar_t type, uchar_t *data, size_t dataSize)
{
    DescriptorFactory& instance = DescriptorFactory::GetInstance();
    return instance.Create(type, data, dataSize);
}

