#include "SystemInclude.h"
#include "Common.h"
#include "Debug.h"

#include "Descriptor.h"

using namespace std;

/**********************class UcharDescriptor**********************/
size_t UcharDescriptor::GetCodesSize() const
{
    return data.get()[1] + 2;
}

uchar_t UcharDescriptor::GetTag() const
{
    return data.get()[0];
}

size_t UcharDescriptor::MakeCodes(uchar_t *buffer, size_t bufferSize) const
{
    assert(GetCodesSize() <= bufferSize);
    MemCopy(buffer, bufferSize, data.get(), GetCodesSize());
    return GetCodesSize();
}

void UcharDescriptor::Put(std::ostream& os) const
{
    ios::fmtflags flags = cout.flags( );
    os << "Descriptor hex code: ";
    for (uchar_t i = 0; i < GetCodesSize(); ++i)
    {
        os << hex << setw(2) << setfill('0') << (int)data.get()[i];
    }
    os << endl;

    cout.flags(flags);
}

/**********************class Descriptors**********************/
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

