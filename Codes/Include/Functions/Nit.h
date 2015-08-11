#ifndef _Nit_h_
#define _Nit_h_

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
class NitTransportStream: public SectionBase
{
public:
    NitTransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;    
    
    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uint16_t transportStreamId;
    uint16_t originalNetworkId;
};

/**********************class Nit**********************/
class Nit: public SectionBase
{
public:
    typedef NitTransportStream TransportStream;
    Nit();
    ~Nit() {}
    
    uint16_t GetPid()  const; 

    void SetTableId(uchar_t data);
    void SetNetworkId(uint16_t data);
    void SetVersionNumber(uchar_t data);
    TransportStream& AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uchar_t  tableId;
    uint16_t networkId;
    uchar_t  versionNumber;
    std::list<std::shared_ptr<TransportStream>> transportStreams;
};

#endif