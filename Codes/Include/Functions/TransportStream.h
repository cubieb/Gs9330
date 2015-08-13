#ifndef _TransportStream_h_
#define _TransportStream_h_

#include "Component.h"

class Descriptors;

/**********************class TransportStream**********************/
class TransportStream: public Component
{
public:
    TransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId);

    void AddDescriptor(uchar_t tag, uchar_t* data, size_t dataSize);
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
class TransportStreams: public Component
{    
public:
    void AddTransportStream(uint16_t theTransportStreamId, uint16_t theOriginalNetworkId);
    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    std::list<std::shared_ptr<TransportStream>> transportStreams;
};

#endif