#ifndef _Descriptor_h_
#define _Descriptor_h_

class Discriptor
{
public:
    Discriptor() {}
    virtual ~Discriptor() {}

    virtual size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const = 0;
};

class UcharDescriptor: public Discriptor
{
public: 
    enum: uchar_t {Tag  = 0x40};
    UcharDescriptor(uchar_t *theData, size_t theDataSize)
        : data(new uchar_t[theDataSize], UcharDeleter()), pointer(0)
    {}

    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const
    {
        return 0;
    }

private:
    std::shared_ptr<uchar_t> data;
    size_t pointer;
};

typedef UcharDescriptor NetworkNameDescriptor;

class DescriporFactory
{
public:
    Discriptor* operator()(uchar_t tag, uchar_t *data, size_t dataSize) const
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
};

#endif