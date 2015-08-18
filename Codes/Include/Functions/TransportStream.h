#ifndef _TransportStream_h_
#define _TransportStream_h_

#include "Component.h"

class Descriptors;

/**********************class TransportStream**********************/
//5.2.1 Network Information Table (NIT)
class TransportStream: public Component
{
public:
    TransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId);

    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
    uint16_t GetTsId() { return transportStreamId; }
    uint16_t GetOnId() { return originalNetworkId; }

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;    
    
    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uint16_t transportStreamId;
    uint16_t originalNetworkId;
    std::shared_ptr<Descriptors> descriptors;
};

/**********************class TransportStreams**********************/
class TransportStreams: public Components
{    
public:
    typedef Components MyBase;

    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    void AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId);
    void AddTsDescriptor(uint16_t tsId, uint16_t onId, uchar_t tag, uchar_t* data, size_t dataSize);

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;
};

#endif