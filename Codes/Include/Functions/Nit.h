#ifndef _Nit_h_
#define _Nit_h_

class Discriptor;
class NitTransportStream
{
public:
    void AddDescriptor();

private:
    uint16_t transportStreamId;
    uint16_t originalNetworkId;
    std::list<Discriptor*> discripters;
};

class Nit
{
public:
    typedef NitTransportStream TransportStream;
    
    void SetTableId(uchar_t value);
    void SetNetworkId(uint16_t value);
    void SetVersionNumber(uchar_t value);
    void AddDescriptor(uchar_t value);
    TransportStream AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId);

    void MakeSection();

private:
    std::list<Discriptor*> discripters;
};

#endif