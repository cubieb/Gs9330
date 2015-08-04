#ifndef _Nit_h_
#define _Nit_h_

class Discriptor;
class NitTransportStream
{
public:
    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);

private:
    uint16_t transportStreamId;
    uint16_t originalNetworkId;
    std::list<std::shared_ptr<Discriptor>> discripters;
};

class Nit
{
public:
    typedef NitTransportStream TransportStream;
    
    void SetTableId(uchar_t data);
    void SetNetworkId(uint16_t data);
    void SetVersionNumber(uchar_t data);
    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    TransportStream AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId);

    void MakeCodes();

private:
    std::list<std::shared_ptr<Discriptor>> discripters;
};

#endif