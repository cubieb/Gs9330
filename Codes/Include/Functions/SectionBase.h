#ifndef _SectionBase_h_
#define _SectionBase_h_

#define MaxNitSectionLength     1024

#define SectionSyntaxIndicator  0x1
#define Reserved1Bit            0x1
#define Reserved2Bit            0x3
#define Reserved4Bit            0xF

class Discriptor;
/**********************class SectionBase**********************/
class SectionBase
{
public:
    virtual uint16_t GetPid()  const; 
    virtual void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);

    virtual size_t GetCodesSize() const;
    virtual size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    virtual int Compare(const SectionBase& right) const;
    virtual void Put(std::ostream& os) const;

protected:
    std::list<std::shared_ptr<Discriptor>> descriptors;
};

inline bool operator==(const SectionBase& left, const SectionBase& right)
{
    return (left.Compare(right) == 0);
}

inline bool operator!=(const SectionBase& left, const SectionBase& right)
{
    return (left.Compare(right) != 0);
}

inline bool operator>(const SectionBase& left, const SectionBase& right)
{
    return (left.Compare(right) > 0);
}

inline bool operator<(const SectionBase& left, const SectionBase& right)
{
    return (left.Compare(right) < 0);
}

inline std::ostream& operator << (std::ostream& os, const SectionBase& nit)
{
    nit.Put(os);
    return os;
}

/**********************class NitTransportStream**********************/
class TransportStream: public SectionBase
{
public:
    TransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;    
    
    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uint16_t transportStreamId;
    uint16_t originalNetworkId;
};

#endif