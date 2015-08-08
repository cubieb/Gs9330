#ifndef _Nit_h_
#define _Nit_h_

class Discriptor;
/**********************class SectionBase**********************/
class SectionBase
{
public:
    virtual void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);

    virtual size_t GetCodesSize() const;
    virtual size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

protected:
    std::list<std::shared_ptr<Discriptor>> discripters;
};

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

inline std::ostream& operator << (std::ostream& os, const NitTransportStream& nitTs)
{
    nitTs.Put(os);
    return os;
}

/**********************class Nit**********************/
class Nit: public SectionBase
{
public:
    typedef NitTransportStream TransportStream;
    Nit();
    ~Nit() {}

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

inline std::ostream& operator << (std::ostream& os, const Nit& nit)
{
    nit.Put(os);
    return os;
}

#endif