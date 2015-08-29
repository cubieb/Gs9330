#ifndef _Descriptor_h_
#define _Descriptor_h_

#include "Component.h"
/**********************class UcharDescriptor**********************/
class UcharDescriptor: public Descriptor
{
public: 
    UcharDescriptor(size_t theDataSize)
        : data(new uchar_t[theDataSize+1], UcharDeleter()), dataSize(theDataSize)
    {
    }

    UcharDescriptor(uchar_t *theData, size_t theDataSize)
        : data(new uchar_t[theDataSize+1], UcharDeleter()), dataSize(theDataSize)
    {
        /* we allocate 1 more bytes to store '0', just for debug function Put() to
           show the value correctly.
         */
        data.get()[theDataSize] = '\0';
        memcpy(data.get(), theData, theDataSize);
    }

    virtual ~UcharDescriptor()
    {}

    virtual uchar_t GetTag() const = 0;
    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

protected:
    std::shared_ptr<uchar_t> data;
    size_t dataSize;
};

/**********************class NetworkNameDescriptor**********************/
/* network_name_descriptor */
class NetworkNameDescriptor: public UcharDescriptor
{
public:
    enum: uchar_t {Tag  = 0x40};
    NetworkNameDescriptor(uchar_t *data, size_t dataSize)
        : UcharDescriptor(data, dataSize)
    {}

    static Descriptor* CreateInstance(uchar_t *data, size_t dataSize)
    {
        return new NetworkNameDescriptor(data, dataSize);
    }
    uchar_t GetTag() const { return Tag; }
};

/**********************class ServiceListDescriptor**********************/
/* service_list_descriptor */
class ServiceListDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x41};
    typedef Descriptor*(*Constructor1)(uchar_t *, size_t);
    typedef Descriptor*(*Constructor2)(const std::list<std::pair<uint16_t, uchar_t>>&);

    ServiceListDescriptor(uchar_t *data, size_t dataSize)
        : UcharDescriptor(data, dataSize)
    {}

    ServiceListDescriptor(const std::list<std::pair<uint16_t, uchar_t>>& serviceList)
        : UcharDescriptor(3 * serviceList.size())
    {
        uchar_t *ptr = data.get();
        for (auto iter: serviceList)
        {
            ptr = ptr + Write16(ptr, iter.first);
            ptr = ptr + Write8(ptr, iter.second);
        }
    }

    static Descriptor* CreateInstance(const std::list<std::pair<uint16_t, uchar_t>>& serviceList)
    {
        return new ServiceListDescriptor(serviceList);
    }

    static Descriptor* CreateInstance(uchar_t *data, size_t dataSize)
    {
        return new ServiceListDescriptor(data, dataSize);
    }
    uchar_t GetTag() const { return Tag; }
};

/**********************class StuffingDescriptor**********************/
/* stuffing_descriptor */
class StuffingDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x42};
    StuffingDescriptor(uchar_t *data, size_t dataSize)
        : UcharDescriptor(data, dataSize)
    {}

    static Descriptor* CreateInstance(uchar_t *data, size_t dataSize)
    {
        return new StuffingDescriptor(data, dataSize);
    }
    uchar_t GetTag() const { return Tag; }
};

/**********************class SatelliteDeliverySystemDescriptor**********************/
/* satellite_delivery_system_descriptor */
class SatelliteDeliverySystemDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x43};
    SatelliteDeliverySystemDescriptor(uchar_t *data, size_t dataSize)
        : UcharDescriptor(data, dataSize)
    {}

    static Descriptor* CreateInstance(uchar_t *data, size_t dataSize)
    {
        return new SatelliteDeliverySystemDescriptor(data, dataSize);
    }
    uchar_t GetTag() const { return Tag; }
};

/**********************class CableDeliverySystemDescriptor**********************/
/* cable_delivery_system_descriptor  */
class CableDeliverySystemDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x44};
    typedef Descriptor*(*Constructor1)(uchar_t *, size_t);
    typedef Descriptor*(*Constructor2)(uint32_t, uint16_t, uchar_t, uint32_t, uint32_t);

    CableDeliverySystemDescriptor(uchar_t *data, size_t dataSize)
        : UcharDescriptor(data, dataSize)
    {}
    
    CableDeliverySystemDescriptor(uint32_t frequency, uint16_t fecOuter, uchar_t modulation,
                                  uint32_t symbolRate, uint32_t fecInner)
        : UcharDescriptor(11)
    {
        uchar_t *ptr = data.get();
        frequency = frequency * 1000 / 100; //KHz to Hz, Minimal accurace is 100 Hz
        for (int i = 0; i < 4; i++)
        {
            ptr[3 - i] = ConvertValueToBcd(frequency % 100);
            frequency = frequency / 100;
        }
        //ptr = ptr + Write32(ptr, frequency);
        ptr = ptr + 4;
        ptr = ptr + Write16(ptr, (Reserved12Bit << 4) | (fecOuter & 0xf));
        ptr = ptr + Write8(ptr, modulation);

        symbolRate = symbolRate * 1000 / 10; //KHz to Hz, Minimal accurace is 10 Hz
        for (int i = 0; i < 4; i++)
        {
            ptr[3 - i] = ConvertValueToBcd(symbolRate % 100);
            symbolRate = symbolRate / 100;
        }
        ptr[3] = (ptr[3] & 0xf0) | fecInner;
    }

    static Descriptor* CreateInstance(uint32_t frequency, uint16_t fecOuter, uchar_t modulation,
                                      uint32_t symbolRate, uint32_t fecInner)
    {
        return new CableDeliverySystemDescriptor(frequency, fecOuter, modulation, symbolRate, fecInner);
    }

    static Descriptor* CreateInstance(uchar_t *data, size_t dataSize)
    {
        return new CableDeliverySystemDescriptor(data, dataSize);
    }
    uchar_t GetTag() const { return Tag; }
};

/**********************class ServiceDescriptor**********************/
/* service_descriptor  */
class ServiceDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x48};
    typedef Descriptor*(*Constructor1)(uchar_t *, size_t);
    typedef Descriptor*(*Constructor2)(uchar_t, uchar_t*, uchar_t*);

    ServiceDescriptor(uchar_t *data, size_t dataSize)
        : UcharDescriptor(data, dataSize)
    {}
    
    ServiceDescriptor(uchar_t serviceType, uchar_t *providerName, uchar_t *serviceName)
        : UcharDescriptor(3 + strlen((char*)providerName) + strlen((char*)serviceName))
    {
        uchar_t *ptr = data.get();
        ptr = ptr + Write8(ptr, serviceType);

        size_t providerNameSize = strlen((char*)providerName);
        size_t serviceNameSize  = strlen((char*)serviceName);

        ptr = ptr + Write8(ptr, providerNameSize);
        ptr = ptr + MemCopy(ptr, dataSize - 2, providerName, providerNameSize);
                
        ptr = ptr + Write8(ptr, serviceNameSize);
        ptr = ptr + MemCopy(ptr, dataSize - 3 - providerNameSize, serviceName, serviceNameSize);
    }

    static Descriptor* CreateInstance(uchar_t serviceType, uchar_t *providerName, uchar_t *serviceName)
    {
        return new ServiceDescriptor(serviceType, providerName, serviceName);
    }

    static Descriptor* CreateInstance(uchar_t *data, size_t dataSize)
    {
        return new ServiceDescriptor(data, dataSize);
    }
    uchar_t GetTag() const { return Tag; }
};

/**********************class NetworkNameDescriptor**********************/
/* bouquet_name_descriptor */
class BouquetNameDescriptor: public UcharDescriptor
{
public:
    enum: uchar_t {Tag  = 0x47};
    BouquetNameDescriptor(uchar_t *data, size_t dataSize)
        : UcharDescriptor(data, dataSize)
    {}

    static Descriptor* CreateInstance(uchar_t *data, size_t dataSize)
    {
        return new BouquetNameDescriptor(data, dataSize);
    }
    uchar_t GetTag() const { return Tag; }
};

/**********************class UserdefinedDscriptor83**********************/
/* user defined dscriptor  */
class UserdefinedDscriptor83: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x83};
    UserdefinedDscriptor83(uchar_t *data, size_t dataSize)
        : UcharDescriptor(data, dataSize)
    {}

    static Descriptor* CreateInstance(uchar_t *data, size_t dataSize)
    {
        return new UserdefinedDscriptor83(data, dataSize);
    }
    uchar_t GetTag() const { return Tag; }
};

/**********************class Descriptors**********************/
class Descriptors: public Components
{
public:
    typedef Components MyBase;

    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    void AddDescriptor0x41(const std::list<std::pair<uint16_t, uchar_t>>& serviceList);
    void AddDescriptor0x44(uint32_t frequency, uint16_t fecOuter, uchar_t modulation,
                           uint32_t symbolRate, uint32_t fecInner);
    void AddDescriptor0x48(uchar_t serviceType, uchar_t *providerName, uchar_t *serviceName);

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;
};

/**********************class DescriptorFactor**********************/
typedef std::function<Descriptor*(uchar_t *data, size_t dataSize)> DescriptorCreator;

class DescriptorFactory
{
public:
    void Register(uchar_t type, DescriptorCreator creator);
    Descriptor* Create(uchar_t type, uchar_t *data, size_t dataSize);

    static DescriptorFactory& GetInstance()
    {
        static DescriptorFactory instance;
        return instance;
    }

private:
    DescriptorFactory() { /* do nothing */ }
    std::map<uchar_t, DescriptorCreator> creators;
};

class AutoRegisterSuite
{
public:
    AutoRegisterSuite(uchar_t type, DescriptorCreator creator)
    {
        DescriptorFactory& factory = DescriptorFactory::GetInstance();
        factory.Register(type, creator);
    }
};

#define DescriptorCreatorRgistration(type, creator)      \
    static AutoRegisterSuite  JoinName(descriptorCreator, __LINE__)(type, creator)

Descriptor* CreateDescriptor(uchar_t type, uchar_t *data, size_t dataSize);

#endif