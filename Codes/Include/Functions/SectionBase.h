#ifndef _SectionBase_h_
#define _SectionBase_h_

class Descriptor;
class Descriptors;
/**********************class SectionBase**********************/
class SectionBase
{
public:
    virtual uint16_t GetPid()  const = 0; 

    virtual size_t GetCodesSize() const = 0;
    virtual size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const = 0;

    /* the following function is provided just for debug */
    int Compare(const SectionBase& right) const;
    virtual void Put(std::ostream& os) const = 0;

protected:
    std::list<std::shared_ptr<Descriptor>> descriptors;
};
GenerateInlineCodes(SectionBase);

/**********************class TransportStream**********************/
class TransportStream
{
public:
    TransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId);

    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;    
    
    /* the following function is provided just for debug */
    int Compare(const TransportStream& right) const;
    void Put(std::ostream& os) const;

private:
    uint16_t transportStreamId;
    uint16_t originalNetworkId;
    Descriptors descriptors;
};
GenerateInlineCodes(TransportStream);

#endif