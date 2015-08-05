#ifndef _Nit_h_
#define _Nit_h_

class Discriptor;
/**********************class NitTransportStream**********************/
class NitTransportStream
{
public:
    NitTransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId);
    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;    

private:
    uint16_t transportStreamId;
    uint16_t originalNetworkId;
    std::list<std::shared_ptr<Discriptor>> discripters;
};

/**********************class Nit**********************/
class Nit
{
public:
    typedef NitTransportStream TransportStream;
    Nit();
    ~Nit() {}

    void SetTableId(uchar_t data);
    void SetNetworkId(uint16_t data);
    void SetVersionNumber(uchar_t data);
    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    TransportStream& AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId);

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

private:
    uchar_t  tableId;
    uint16_t networkId;
    uchar_t  versionNumber;
    uchar_t  sectionNumber;
    std::list<std::shared_ptr<TransportStream>> transportStreams;
    std::list<std::shared_ptr<Discriptor>> discripters;
};

#endif