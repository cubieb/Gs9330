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
    void AddDescriptor0x41(const std::list<std::pair<uint16_t, uchar_t>>& serviceList);
    void AddDescriptor0x44(uint32_t frequency, uint16_t fecOuter, uchar_t modulation,
                           uint32_t symbolRate, uint32_t fecInner);

    uint16_t GetTsId() const { return transportStreamId; }
    uint16_t GetOnId() const { return originalNetworkId; }

    size_t GetCodesSize() const;
    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;    
    
    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;

private:
    uint16_t transportStreamId;
    uint16_t originalNetworkId;
    std::shared_ptr<Descriptors> descriptors;
};

class EqualTs: std::unary_function<const std::shared_ptr<Component>&, bool>
{
public:
    EqualTs(uint16_t tsId)
        : transportStreamId(tsId)
    {}

    result_type operator()(argument_type component)
    {
        TransportStream& ts = dynamic_cast<TransportStream&>(*component);
        return (result_type)(ts.GetTsId() == transportStreamId);
    }

private:
    uint16_t transportStreamId;
};

/**********************class TransportStreams**********************/
class TransportStreams: public Components
{    
public:
    typedef Components MyBase;

    size_t MakeCodes(uchar_t *buffer, size_t bufferSize) const;

    void AddTransportStream(uint16_t transportStreamId, uint16_t originalNetworkId);
    void AddTsDescriptor(uint16_t tsId, uchar_t tag, uchar_t* data, size_t dataSize);
    void AddTsDescriptor0x41(uint16_t tsId,
                             const std::list<std::pair<uint16_t, uchar_t>>& serviceList);
    void AddTsDescriptor0x44(uint16_t tsId,
                             uint32_t frequency, uint16_t fecOuter, uchar_t modulation,
                             uint32_t symbolRate, uint32_t fecInner);

    /* the following function is provided just for debug */
    void Put(std::ostream& os) const;
};

#endif