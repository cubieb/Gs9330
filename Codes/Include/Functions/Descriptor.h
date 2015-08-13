#ifndef _Descriptor_h_
#define _Descriptor_h_

#include "Component.h"
/**********************class UcharDescriptor**********************/
class UcharDescriptor: public Descriptor
{
public: 
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

    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;
    size_t GetCodesSize() const;

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    std::shared_ptr<uchar_t> data;
    size_t dataSize;
};

/**********************class NetworkNameDescriptor**********************/
/* network_name_descriptor */
class NetworkNameDescriptor: public UcharDescriptor
{
public:
    enum: uchar_t {Tag  = 0x40};
    NetworkNameDescriptor(uchar_t *theData, size_t theDataSize)
        : UcharDescriptor(theData, theDataSize)
    {}
};

/**********************class ServiceListDescriptor**********************/
/* service_list_descriptor */
class ServiceListDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x41};
    ServiceListDescriptor(uchar_t *theData, size_t theDataSize)
        : UcharDescriptor(theData, theDataSize)
    {}
};

/**********************class StuffingDescriptor**********************/
/* stuffing_descriptor */
class StuffingDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x42};
    StuffingDescriptor(uchar_t *theData, size_t theDataSize)
        : UcharDescriptor(theData, theDataSize)
    {}
};

/**********************class SatelliteDeliverySystemDescriptor**********************/
/* satellite_delivery_system_descriptor */
class SatelliteDeliverySystemDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x43};
    SatelliteDeliverySystemDescriptor(uchar_t *theData, size_t theDataSize)
        : UcharDescriptor(theData, theDataSize)
    {}
};

/**********************class Descriptors**********************/
class Descriptors: public Component
{
public:
    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    std::list<std::shared_ptr<Descriptor>> descriptors;
};

/**********************class DescriporFactory**********************/
class DescriporFactory
{
public:
    Descriptor* Create(uchar_t tag, uchar_t *data, size_t dataSize) const
    {
        return CreateA<0x0>(data, dataSize);
    }

private:
    template<uchar_t Tag>
    Descriptor* CreateA(uchar_t *data, size_t dataSize) const
    {
        Descriptor* descriptor = CreateB<Tag>(data, dataSize);
        if (descriptor != nullptr)
            return descriptor;

        return CreateA<Tag + 1>(data, dataSize);        
    }

    template<>
    Descriptor* CreateA<0xFF>(uchar_t *data, size_t dataSize) const
    {
        return nullptr;       
    }

    template<uchar_t Tag>
    Descriptor* CreateB(uchar_t *data, size_t dataSize) const
    {
        return nullptr;
    }

    template<>
    Descriptor* CreateB<NetworkNameDescriptor::Tag>(uchar_t *data, size_t dataSize) const
    {
        return new NetworkNameDescriptor(data, dataSize);
    }

    template<>
    Descriptor* CreateB<ServiceListDescriptor::Tag>(uchar_t *data, size_t dataSize) const
    {
        return new ServiceListDescriptor(data, dataSize);
    }
    
    template<>
    Descriptor* CreateB<StuffingDescriptor::Tag>(uchar_t *data, size_t dataSize) const
    {
        return new StuffingDescriptor(data, dataSize);
    }
    
    template<>
    Descriptor* CreateB<SatelliteDeliverySystemDescriptor::Tag>(uchar_t *data, size_t dataSize) const
    {
        return new SatelliteDeliverySystemDescriptor(data, dataSize);
    }
};

#endif