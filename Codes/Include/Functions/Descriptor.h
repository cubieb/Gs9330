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

    /* the following function is provided just for debug */
    virtual int Compare(const Discriptor& right) const;
    virtual void Put(std::ostream& os) const = 0;
};

inline bool operator==(const Discriptor& left, const Discriptor& right)
{
    return (left.Compare(right) == 0);
}

inline bool operator!=(const Discriptor& left, const Discriptor& right)
{
    return (left.Compare(right) != 0);
}

inline bool operator>(const Discriptor& left, const Discriptor& right)
{
    return (left.Compare(right) > 0);
}

inline bool operator<(const Discriptor& left, const Discriptor& right)
{
    return (left.Compare(right) < 0);
}

inline std::ostream& operator << (std::ostream& os, const Discriptor& descriptor)
{
    descriptor.Put(os);
    return os;
}

/**********************class UcharDescriptor**********************/
class UcharDescriptor: public Discriptor
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