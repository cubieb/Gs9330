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
void Descriptors::AddDescriptor(uchar_t tag, uchar_t* data)
{
    Descriptor* ptr = CreateDescriptor(tag, data);
    if (ptr == nullptr)
    {
        cout << "we do not support descriptor whose tag = " << hex << (int)tag << endl;
        return;
    }

    shared_ptr<Descriptor> discriptor(ptr);
    AddComponent(discriptor);
}

void Descriptors::AddDescriptor(std::shared_ptr<Descriptor> discriptor)
{
    AddComponent(discriptor);
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
DescriptorCreatorRegistration(NetworkNameDescriptor::Tag, NetworkNameDescriptorCreator);
DescriptorCreatorRegistration(ServiceListDescriptor::Tag, ServiceListDescriptorCreator);
DescriptorCreatorRegistration(StuffingDescriptor::Tag, StuffingDescriptorCreator);
DescriptorCreatorRegistration(SatelliteDeliverySystemDescriptor::Tag, SatelliteDeliverySystemDescriptorCreator);
DescriptorCreatorRegistration(CableDeliverySystemDescriptor::Tag, CableDeliverySystemDescriptorCreator);
DescriptorCreatorRegistration(BouquetNameDescriptor::Tag, BouquetNameDescriptorCreator);
DescriptorCreatorRegistration(ServiceDescriptor::Tag, ServiceDescriptorCreator);
DescriptorCreatorRegistration(ShortEventDescriptor::Tag, ShortEventDescriptorCreator);
DescriptorCreatorRegistration(ExtendedEventDescriptor::Tag, ExtendedEventDescriptorCreator);
DescriptorCreatorRegistration(UserdefinedDscriptor83::Tag, UserdefinedDscriptor83Creator);

/* Create descriptor from string */
Descriptor* CreateDescriptor(uchar_t tag, uchar_t *data)
{
    DescriptorFactory& instance = DescriptorFactory::GetInstance();
    return instance.Create(tag, data);
}

Descriptor* CreateDescriptor(uchar_t tag, uchar_t *data, size_t dataSize)
{
    DescriptorFactory& instance = DescriptorFactory::GetInstance();
    return instance.Create(tag, data, dataSize);
}

