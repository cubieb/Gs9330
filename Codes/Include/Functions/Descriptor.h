#ifndef _Descriptor_h_
#define _Descriptor_h_

/**********************class Discriptor**********************/
class Discriptor
{
public:
    Discriptor() {}
    virtual ~Discriptor() {}

    virtual size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const = 0;
    virtual size_t GetCodesSize() const = 0;
};

/**********************class UcharDescriptor**********************/
class UcharDescriptor: public Discriptor
{
public: 
    UcharDescriptor(uchar_t *theData, size_t theDataSize)
        : data(new uchar_t[theDataSize], UcharDeleter()), dataSize(theDataSize)
    {
        memcpy(data.get(), theData, theDataSize);
    }

    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const
    {
        assert(dataSize <= bufferSize);
        memcpy(buffer, data.get(), dataSize);
        return dataSize;
    }

    size_t GetCodesSize() const
    {
        return dataSize;
    }

private:
    std::shared_ptr<uchar_t> data;
    size_t dataSize;
};

/**********************class NetworkNameDescriptor**********************/
class NetworkNameDescriptor: public UcharDescriptor
{
public:
    enum: uchar_t {Tag  = 0x40};
    NetworkNameDescriptor(uchar_t *theData, size_t theDataSize)
        : UcharDescriptor(theData, theDataSize)
    {}
};

/**********************class ServiceListDescriptor**********************/
class ServiceListDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x41};
    ServiceListDescriptor(uchar_t *theData, size_t theDataSize)
        : UcharDescriptor(theData, theDataSize)
    {}
};

/**********************class StuffingDescriptor**********************/
class StuffingDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x42};
    StuffingDescriptor(uchar_t *theData, size_t theDataSize)
        : UcharDescriptor(theData, theDataSize)
    {}
};

/**********************class SatelliteDeliverySystemDescriptor**********************/
class SatelliteDeliverySystemDescriptor: public UcharDescriptor
{
public: 
    enum: uchar_t {Tag  = 0x43};
    SatelliteDeliverySystemDescriptor(uchar_t *theData, size_t theDataSize)
        : UcharDescriptor(theData, theDataSize)
    {}
};

/**********************class DescriporFactory**********************/
class DescriporFactory
{
public:
    Discriptor* Create(uchar_t tag, uchar_t *data, size_t dataSize) const
    {
        return CreateA<0x0>(data, dataSize);
    }

private:
    template<uchar_t Tag>
    Discriptor* CreateA(uchar_t *data, size_t dataSize) const
    {
        Discriptor* descriptor = CreateB<Tag>(data, dataSize);
        if (descriptor != nullptr)
            return descriptor;

        return CreateA<Tag + 1>(data, dataSize);        
    }

    template<>
    Discriptor* CreateA<0xFF>(uchar_t *data, size_t dataSize) const
    {
        return nullptr;       
    }

    template<uchar_t Tag>
    Discriptor* CreateB(uchar_t *data, size_t dataSize) const
    {
        return nullptr;
    }

    template<>
    Discriptor* CreateB<NetworkNameDescriptor::Tag>(uchar_t *data, size_t dataSize) const
    {
        return new NetworkNameDescriptor(data, dataSize);
    }

    template<>
    Discriptor* CreateB<ServiceListDescriptor::Tag>(uchar_t *data, size_t dataSize) const
    {
        return new ServiceListDescriptor(data, dataSize);
    }
    
    template<>
    Discriptor* CreateB<StuffingDescriptor::Tag>(uchar_t *data, size_t dataSize) const
    {
        return new StuffingDescriptor(data, dataSize);
    }
    
    template<>
    Discriptor* CreateB<SatelliteDeliverySystemDescriptor::Tag>(uchar_t *data, size_t dataSize) const
    {
        return new SatelliteDeliverySystemDescriptor(data, dataSize);
    }
};

#endif